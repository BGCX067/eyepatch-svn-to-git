#include "precomp.h"
#include "constants.h"
#include "ClassifierTester.h"

ClassifierTester::ClassifierTester() {
	CvSize size = cvSize(QUICKTEST_X*GUESSMASK_WIDTH, QUICKTEST_Y*GUESSMASK_HEIGHT);
	quickTestImage = cvCreateImage(size, IPL_DEPTH_8U, 3);
	quickTestBitmap = new Bitmap(QUICKTEST_X*GUESSMASK_WIDTH, QUICKTEST_Y*GUESSMASK_HEIGHT, PixelFormat24bppRGB);
}

ClassifierTester::~ClassifierTester() {
	cvReleaseImage(&quickTestImage);
	delete quickTestBitmap;
}

void ClassifierTester::TestClassifierOnVideo(Classifier *c, CVideoLoader *vl, int recognizerMode) {
    HCURSOR hOld = SetCursor(LoadCursor(0, IDC_WAIT));

	int nTestFrames = QUICKTEST_X * QUICKTEST_Y;
	if (!vl->videoLoaded) return;
	int originalFrame = vl->currentFrameNumber;

	if (nTestFrames > vl->nFrames) nTestFrames = vl->nFrames;
	int frameskip = floor( ((float)vl->nFrames) / ((float)nTestFrames) );

	for (int i=0; i< nTestFrames; i++) {
		vl->LoadFrame(i*frameskip);
		int xpos = GUESSMASK_WIDTH*(i % QUICKTEST_X);
		int ypos = GUESSMASK_HEIGHT*(i / QUICKTEST_X);
		CvRect rect = cvRect(xpos, ypos, GUESSMASK_WIDTH, GUESSMASK_HEIGHT);
		cvSetImageROI(quickTestImage, rect);
		RunClassifierOnCurrentFrame(c, vl, recognizerMode);
	}

	cvResetImageROI(quickTestImage);
	SetCursor(hOld);
	DoModal();

	// Restore original frame position
	vl->LoadFrame(originalFrame);
}

void ClassifierTester::RunClassifierOnCurrentFrame(Classifier *classifier, CVideoLoader *vl, int recognizerMode) {

	ClassifierOutputData outdata;

    if (recognizerMode == MOTION_FILTER) {
        outdata = ((MotionClassifier*)classifier)->ClassifyMotion(vl->GetMotionHistory(), MOTION_NUM_HISTORY_FRAMES);
    } else if (recognizerMode == GESTURE_FILTER) {
		MotionTrack mt = vl->GetTrajectoryAtCurrentFrame();
		outdata = ((GestureClassifier*)classifier)->ClassifyTrack(mt);
    } else {
        outdata = classifier->ClassifyFrame(vl->copyFrame);
    }

	IplImage *displaySmall = cvCreateImage(cvSize(GUESSMASK_WIDTH, GUESSMASK_HEIGHT), IPL_DEPTH_8U, 3);
	IplImage *displayMasked = cvCreateImage(cvSize(GUESSMASK_WIDTH, GUESSMASK_HEIGHT), IPL_DEPTH_8U, 3);
	cvZero(displayMasked);
	if (outdata.HasVariable("Mask")) {
		IplImage *mask = outdata.GetImageData("Mask");
		cvResize(vl->copyFrame, displaySmall);
		cvCopy(displaySmall, displayMasked, mask);
	}
	cvAddWeighted(displaySmall, 0.5, displayMasked, 0.5, 0, quickTestImage);

	cvReleaseImage(&displaySmall);
	cvReleaseImage(&displayMasked);
}

LRESULT ClassifierTester::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	ShowWindow(FALSE);	// start with window hidden
	MoveWindow(0,0,QUICKTEST_X*GUESSMASK_WIDTH, QUICKTEST_Y*GUESSMASK_HEIGHT+100, FALSE);
	GetDlgItem(IDOK).MoveWindow(QUICKTEST_X*GUESSMASK_WIDTH/2 - 75, QUICKTEST_Y*GUESSMASK_HEIGHT + 20, 150, 50, FALSE);
	CenterWindow();
	ShowWindow(TRUE);
	return TRUE;    // let the system set the focus
}

LRESULT ClassifierTester::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	EndDialog(IDOK);
	return 0;
}

LRESULT ClassifierTester::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

LRESULT ClassifierTester::OnPaint( UINT, WPARAM, LPARAM, BOOL& )
{
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
	Graphics graphics(hdc);

	IplToBitmap(quickTestImage, quickTestBitmap);	
	graphics.DrawImage(quickTestBitmap, 0, 0);
    EndPaint(&ps);
    return 0;
}

LRESULT ClassifierTester::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {	
	EndDialog(IDOK);
    return 0;
}
