#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"

// Constructor for a standard training sample (no motion or range data)
TrainingSample::TrainingSample(IplImage* srcImage, HWND lc, HIMAGELIST il, Rect bounds, int groupId) {
	// this constructor should only be called for positive and negative sample types
	assert((groupId == GROUPID_POSSAMPLES) || (groupId == GROUPID_NEGSAMPLES));

	hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;
	iOrigId = groupId;
	selectBounds = bounds;
    motionTrack.clear();
	motionHistory = NULL;

    fullImageCopy = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_8U, 3); 
	resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    cvSetImageROI(srcImage, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(srcImage, fullImageCopy);

    if (srcImage->width >= LISTVIEW_SAMPLE_X && srcImage->height >= LISTVIEW_SAMPLE_Y) {
        cvResize(srcImage, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(srcImage, resizedImage, CV_INTER_LINEAR);
    }
    cvResetImageROI(srcImage);

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);

    // Add image to imagelist
    int imgIndex = ImageList_Add(hImageList, hbmImage, NULL);

    // Add item to list view
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iGroupId = groupId;
    lvi.iItem = imgIndex;
    lvi.iImage = imgIndex;
    lvi.iSubItem = 0;
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

// Constructor for motion training sample (includes motion history image)
TrainingSample::TrainingSample(IplImage* srcImage, IplImage* motionHist, HWND lc, HIMAGELIST il, Rect bounds, int groupId) {
	// this constructor should only be called for motion sample type
	assert(groupId == GROUPID_MOTIONSAMPLES);

	hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;
	iOrigId = groupId;
	selectBounds = bounds;
    motionTrack.clear();

    fullImageCopy = cvCreateImage(cvSize(bounds.Width,bounds.Height),IPL_DEPTH_8U, 3); 
    motionHistory = cvCreateImage(cvSize(motionHist->width,motionHist->height),IPL_DEPTH_32F, 1);
	resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    cvSetImageROI(srcImage, cvRect( bounds.X, bounds.Y, bounds.Width, bounds.Height));
    cvCopyImage(srcImage, fullImageCopy);

    if (srcImage->width >= LISTVIEW_SAMPLE_X && srcImage->height >= LISTVIEW_SAMPLE_Y) {
        cvResize(srcImage, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(srcImage, resizedImage, CV_INTER_LINEAR);
    }
    cvResetImageROI(srcImage);

	// copy entire frame motion history image (motion history analysis doesn't work as well on partial frame)
	cvCopyImage(motionHist, motionHistory);

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);

    // Add image to imagelist
    int imgIndex = ImageList_Add(hImageList, hbmImage, NULL);

    // Add item to list view
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iGroupId = groupId;
    lvi.iItem = imgIndex;
    lvi.iImage = imgIndex;
    lvi.iSubItem = 0;
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

// Constructor for range sample (includes motion track)
TrainingSample::TrainingSample(IplImage *frame, MotionTrack mt, HWND lc, HIMAGELIST il, int groupId) {
	// this constructor should only be called for range sample type
	assert(groupId == GROUPID_RANGESAMPLES);

	hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;
	iOrigId = groupId;
    motionTrack = mt;
    motionHistory = NULL;

    fullImageCopy = cvCreateImage(cvSize(frame->width,frame->height),IPL_DEPTH_8U, 3);
    cvZero(fullImageCopy);
    cvAddWeighted(frame, 0.5, fullImageCopy, 0.5, 0.0, fullImageCopy);
    
    // draw the trajectory in the sample image
	Template t("", mt);
    DrawTrack(fullImageCopy, t.m_points, CV_RGB(100,255,100), 3, GESTURE_SQUARE_SIZE);

    resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    cvResize(fullImageCopy, resizedImage, CV_INTER_AREA);

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);

    // Add image to imagelist
    int imgIndex = ImageList_Add(hImageList, hbmImage, NULL);

    // Add item to list view
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iGroupId = groupId;
    lvi.iItem = imgIndex;
    lvi.iImage = imgIndex;
    lvi.iSubItem = 0;
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

// Constructor for loading sample from image file
TrainingSample::TrainingSample(char *filename, HWND lc, HIMAGELIST il, int groupId) {
	// this constructor should only be called for positive and negative sample types
	assert((groupId == GROUPID_POSSAMPLES) || (groupId == GROUPID_NEGSAMPLES));
	
	hwndListControl = lc;
    hImageList = il;
    iGroupId = groupId;
	iOrigId = groupId;
    motionTrack.clear();
	motionHistory = NULL;

    fullImageCopy = cvLoadImage(filename, 1);
    if (fullImageCopy == NULL) {
        fullImageCopy = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3);
        cvZero(fullImageCopy);
    }

    resizedImage = cvCreateImage(cvSize(LISTVIEW_SAMPLE_X,LISTVIEW_SAMPLE_Y),IPL_DEPTH_8U, 3); 
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    if (fullImageCopy->width >= LISTVIEW_SAMPLE_X && fullImageCopy->height >= LISTVIEW_SAMPLE_Y) {
        cvResize(fullImageCopy, resizedImage, CV_INTER_AREA);
    } else { 
        cvResize(fullImageCopy, resizedImage, CV_INTER_LINEAR);
    }

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);

    // Add image to imagelist
    int imgIndex = ImageList_Add(hImageList, hbmImage, NULL);

    // Add item to list view
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_GROUPID;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iGroupId = groupId;
    lvi.iItem = imgIndex;
    lvi.iImage = imgIndex;
    lvi.iSubItem = 0;
    int newListItemPos = ListView_InsertItem(hwndListControl, &lvi);

    id = ListView_MapIndexToID(hwndListControl, newListItemPos);
}

// Constructor for cloning an existing sample
TrainingSample::TrainingSample(TrainingSample *toClone) {
    hwndListControl = toClone->hwndListControl;
    hImageList = toClone->hImageList;
    iGroupId = toClone->iGroupId;
	iOrigId = toClone->iOrigId;
	selectBounds = toClone->selectBounds;
	id = toClone->id;
	width = toClone->width;
	height = toClone->height;
	lvi = toClone->lvi;
	motionTrack = toClone->motionTrack;

    fullImageCopy = cvCloneImage(toClone->fullImageCopy);
	if ((iOrigId == GROUPID_MOTIONSAMPLES) && (toClone->motionHistory != NULL)) {
	    motionHistory = cvCloneImage(toClone->motionHistory);
	} else {
		motionHistory = NULL;
	}
	resizedImage = cvCloneImage(toClone->resizedImage);
    bmpImage = new Bitmap(LISTVIEW_SAMPLE_X, LISTVIEW_SAMPLE_Y, PixelFormat24bppRGB);

    IplToBitmap(resizedImage, bmpImage);
    bmpImage->GetHBITMAP(NULL, &hbmImage);
}

TrainingSample::~TrainingSample(void) {
    cvReleaseImage(&fullImageCopy);
    cvReleaseImage(&resizedImage);
    if (motionHistory != NULL) cvReleaseImage(&motionHistory);
    delete bmpImage;
}

void TrainingSample::Save(WCHAR *directory, int index) {
	USES_CONVERSION;
	WCHAR filename[MAX_PATH];

    if (iGroupId == GROUPID_POSSAMPLES) { // positive sample
		wsprintf(filename, L"%s%s%d%s", directory, FILE_POSIMAGE_PREFIX, index, FILE_IMAGE_EXT);
		cvSaveImage(W2A(filename), fullImageCopy);
	} else if (iGroupId == GROUPID_NEGSAMPLES) { // negative sample
		wsprintf(filename, L"%s%s%d%s", directory, FILE_NEGIMAGE_PREFIX, index, FILE_IMAGE_EXT);
		cvSaveImage(W2A(filename), fullImageCopy);
	} else if (iGroupId == GROUPID_MOTIONSAMPLES) { // motion sample
		wsprintf(filename, L"%s%s%d%s", directory, FILE_MOTIMAGE_PREFIX, index, FILE_IMAGE_EXT);
		cvSaveImage(W2A(filename), fullImageCopy);
		wsprintf(filename, L"%s%s%d%s%s", directory, FILE_MOTIMAGE_PREFIX, index, FILE_IMAGE_EXT, FILE_MOTIONIMAGE_EXT);
		SaveTrackToFile(motionTrack, filename);
	} else if (iGroupId == GROUPID_RANGESAMPLES) { // range sample
		wsprintf(filename, L"%s%s%d%s", directory, FILE_RNGIMAGE_PREFIX, index, FILE_IMAGE_EXT);
		cvSaveImage(W2A(filename), fullImageCopy);
		wsprintf(filename, L"%s%s%d%s%s", directory, FILE_RNGIMAGE_PREFIX, index, FILE_IMAGE_EXT, FILE_MOTIONTRACK_EXT);
		SaveTrackToFile(motionTrack, filename);
    }
}

void TrainingSample::Draw(Graphics *g, int x, int y) {
    g->DrawImage(bmpImage, x, y);
}
