#include "precomp.h"
#include "../constants.h"
#include "FlowTracker.h"

CFlowTrackerDialog::CFlowTrackerDialog(FlowTracker *p) :
	videoRect(10, 35, 330, 275), 
	drawRect(10, 35, 320, 240) {
	m_hMutex = NULL;
	parent = p;
}

CFlowTrackerDialog::~CFlowTrackerDialog() {
	if (m_hMutex) CloseHandle(m_hMutex);
}

LRESULT CFlowTrackerDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow();
	m_hMutex = CreateMutex(NULL,FALSE,NULL);
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
	return TRUE;    // let the system set the focus
}

LRESULT CFlowTrackerDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CFlowTrackerDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
	EndDialog(IDCANCEL);
    return 0;
}

LRESULT CFlowTrackerDialog::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
	Graphics graphics(hdc);
	if (parent->bmpVideo != NULL) {
		graphics.DrawImage(parent->bmpVideo, drawRect);
	}
    EndPaint(&ps);
    return 0;
}

LRESULT CFlowTrackerDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WaitForSingleObject(m_hMutex,INFINITE);
	TerminateThread(m_hThread, 0);
	return 0;
}

DWORD WINAPI CFlowTrackerDialog::ThreadCallback(CFlowTrackerDialog* instance) {
	instance->ConvertFrames();
	return 1L;
}

void CFlowTrackerDialog::ConvertFrames() {
	while (1) {
		WaitForSingleObject(m_hMutex,INFINITE);
		if (parent->currentFrame != NULL) {
			parent->ProcessFrame();

            // Redraw dialog window
			InvalidateRect(&videoRect,FALSE);
		} else {
			ReleaseMutex(m_hMutex);
			EndDialog(0);
			return;
		}
		ReleaseMutex(m_hMutex);
   }
}


FlowTracker::FlowTracker() :
	m_FlowTrackerDialog(this) {
    isTrained = false;
}

FlowTracker::~FlowTracker() {

}

void FlowTracker::ProcessFrame() {
    // make sure the image isn't upside down
    if (currentFrame->origin  == IPL_ORIGIN_TL) {
        cvCopy(currentFrame,copyFrame);
    } else {
        cvFlip(currentFrame,copyFrame);
    }

	cvCopy(grcurrentFrame, grlastFrame);
	cvConvertImage(copyFrame, grcurrentFrame);

	// Pick out the good features in the image
	int nFeatures = FLOW_MAX_TRACK_FEATURES;
	cvGoodFeaturesToTrack(grlastFrame, eigimage, tempimage, lastframe_features, &nFeatures, 0.01, 10);

	double mvecx = 0, mvecy = 0, magnitude = 0, npoints = 0;
	if (nFeatures > 0) {

		cvFindCornerSubPix(grlastFrame, lastframe_features, nFeatures, cvSize(5,5), cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03));

		// Run the optical flow algorithm
		cvCalcOpticalFlowPyrLK(grlastFrame, grcurrentFrame, pyramid1, pyramid2,
			lastframe_features, currframe_features, nFeatures,
			cvSize(5,5), 3, found_features, feature_error,
			cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 ), 0);

		cvFindCornerSubPix(grcurrentFrame, currframe_features, nFeatures, cvSize(5,5), cvSize(-1,-1),
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03));

		// Now there is a 1 in found_features wherever two features match up
		// The feature positions are in lastframe_features and frame_features
		// Draw the flow field and average all the flow vectors
		for (int i=0; i<nFeatures; i++) {
			if ((found_features[i] == 0) || (feature_error[i]>FLOW_MAX_ERROR_THRESHOLD)) continue;
			CvPoint p,q;
			p.x = lastframe_features[i].x;
			p.y = lastframe_features[i].y;
			q.x = currframe_features[i].x;
			q.y = currframe_features[i].y;
			cvCircle(copyFrame, p, 2, CV_RGB(100,100,255)); 
			cvLine(copyFrame, p, q, CV_RGB(100,100,255), 1, CV_AA);
			mvecx += (currframe_features[i].x-lastframe_features[i].x);
			mvecy += (currframe_features[i].y-lastframe_features[i].y);
			npoints++;
		}
		if (npoints>0) {
			mvecx /= npoints;
			mvecy /= npoints;
		}
		magnitude = _hypot(mvecx, mvecy);
		if (magnitude < FLOW_MIN_MOTION_THRESHOLD) {
			mvecx = 0;
			mvecy = 0;
			numInactiveFrames++;
		} else {
			numInactiveFrames = 0;
		}

	} else {	// no trackable features were detected (similar to case when motion level is below threshold)
		numInactiveFrames++;
	}

	cvCircle(copyFrame, cvPoint(copyFrame->width/2, copyFrame->height/2), 3, CV_RGB(255,255,255),-1,CV_AA);
	cvLine(copyFrame, cvPoint(copyFrame->width/2, copyFrame->height/2),
		cvPoint(copyFrame->width/2+30*mvecx, copyFrame->height/2+30*mvecy), CV_RGB(255,255,255), 2, CV_AA);

	currentX += mvecx;
	currentY += mvecy;
	OneDollarPoint pt(currentX, currentY);
	trajectory.push_back(pt);

	MotionTrack scaledpts = ScaleToSquare(trajectory, 2*GESTURE_SQUARE_SIZE/3);
	scaledpts = TranslateToOrigin(scaledpts);
	DrawTrack(copyFrame, scaledpts, CV_RGB(100,255,100), 3, GESTURE_SQUARE_SIZE, GESTURE_MAX_TRAJECTORY_LENGTH);

    // display color foreground image in window
    IplToBitmap(copyFrame, bmpVideo);

    // grab the next frame of the video
    currentFrame = cvQueryFrame(videoCapture);
}

void FlowTracker::LearnTrajectories(CvCapture* vidCap) {

    videoCapture = vidCap;
    if (!videoCapture) return;

    int nFrames = cvGetCaptureProperty(videoCapture, CV_CAP_PROP_FRAME_COUNT);
    cvSetCaptureProperty(videoCapture, CV_CAP_PROP_POS_FRAMES, 0);

    // TODO: display an informative error message if there are not enough frames in the video
    if (nFrames < GESTURE_MIN_TRAJECTORY_LENGTH) return;

    currentFrame = cvQueryFrame(videoCapture);
    if(currentFrame == NULL) return;

	// Allocate images to store previous frame
    copyFrame = cvCloneImage(currentFrame);
	int width = currentFrame->width;
	int height = currentFrame->height;

	// Allocate images to store grayscale copies
	grcurrentFrame = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1); 
	cvConvertImage(copyFrame, grcurrentFrame);
	grlastFrame = cvCloneImage(grcurrentFrame); 
	
	// Allocate some arrays needed as workspace for the optical flow algorithm
	eigimage = cvCreateImage(cvSize(width,height),IPL_DEPTH_32F,1);
	tempimage = cvCreateImage(cvSize(width,height),IPL_DEPTH_32F,1);
	pyramid1 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	pyramid2 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);

    // Create a bitmap to display video
    bmpVideo = new Bitmap(currentFrame->width, currentFrame->height, PixelFormat24bppRGB);

	currentX = currentFrame->width/2;
	currentY = currentFrame->height/2;
	numInactiveFrames = 0;

    m_FlowTrackerDialog.DoModal();

    cvReleaseImage(&copyFrame);
	cvReleaseImage(&grcurrentFrame);
	cvReleaseImage(&grlastFrame);
	cvReleaseImage(&eigimage);
	cvReleaseImage(&tempimage);
	cvReleaseImage(&pyramid1);
	cvReleaseImage(&pyramid2);

    isTrained = true;
}

MotionTrack FlowTracker::GetTrajectoryInRange(long startFrame, long endFrame) {
	MotionTrack subtrack;
	if (startFrame < 0) startFrame = 0;
	if (endFrame > trajectory.size()-1) endFrame = trajectory.size()-1;
	if (endFrame - startFrame < GESTURE_MIN_TRAJECTORY_LENGTH) return subtrack;

	for (int i=startFrame; i<=endFrame; i++) {
		subtrack.push_back(trajectory[i]);
	}
	subtrack = ScaleToSquare(subtrack, GESTURE_SQUARE_SIZE);
	return subtrack;
}

MotionTrack FlowTracker::GetTrajectoryAtFrame(long frameNum) {
	MotionTrack subtrack;
	if (frameNum < GESTURE_MIN_TRAJECTORY_LENGTH) return subtrack;
	if (frameNum > trajectory.size()-1) return subtrack;
	int startFrame = max(0, frameNum-GESTURE_MAX_TRAJECTORY_LENGTH);

	for (int i=startFrame; i<=frameNum; i++) {
		subtrack.push_back(trajectory[i]);
	}
	subtrack = ScaleToSquare(subtrack, GESTURE_SQUARE_SIZE);
	return subtrack;
}
