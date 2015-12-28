#include "precomp.h"
#include "Classifier.h"
#include "constants.h"

CClassifierDialog::CClassifierDialog(Classifier* c) { 
	parent = c;
}

CClassifierDialog::~CClassifierDialog() {
}

LRESULT CClassifierDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	USES_CONVERSION;

	ShowWindow(FALSE);	// start with window hidden
	WCHAR label[MAX_PATH];
	wsprintf(label, L"Select which data should be output from the \"%s\" recognizer:", parent->GetName());
	GetDlgItem(IDC_TOP_LABEL).SetWindowText(label);

	int nVariables = parent->outputData.NumVariables();
	int varIdx = 0;
	for (int i=0; i<nVariables; i++) {
		string varName = parent->outputData.GetNameOfIndex(i);
		bool varState = parent->outputData.GetStateOfIndex(i);
		ClassifierVariableType type = parent->outputData.GetTypeOfIndex(i);
		if ((type != CVAR_IMAGE) && (type !=CVAR_SEQ)) {	// we don't currently have a standard way to output images 
			CWindow checkbox;
			checkbox.Create(L"BUTTON", this->m_hWnd, CRect(10,50+varIdx*25,300,50+varIdx*25+25),
				A2W(varName.c_str()), WS_CHILD | BS_AUTOCHECKBOX );
			Button_SetCheck(checkbox, varState);
			checkbox.ShowWindow(true);
			checkbox.UpdateWindow();
			varIdx++;
		}
	}

	MoveWindow(0,0,300, varIdx*25+170, FALSE);
	GetDlgItem(IDOK).MoveWindow(100, varIdx*25+80, 100, 50, FALSE);
	CenterWindow();
	ShowWindow(TRUE);	// now that we're done updating, show the newly setup window

	return TRUE;    // let the system set the focus
}

LRESULT CClassifierDialog::OnButtonClicked(UINT uMsg, WPARAM wParam, HWND hwndButton, BOOL& bHandled) {
	if (LOWORD(wParam) == IDOK) {	// the clicked button was the dismiss dialog button
		EndDialog(IDOK);
		return FALSE;
	}

	WCHAR buttonName[MAX_PATH];
	::Button_GetText(hwndButton, buttonName, MAX_PATH);
	if (Button_GetCheck(hwndButton) == BST_CHECKED) {	// activate corresponding variable
		parent->ActivateVariable(buttonName, TRUE);
	} else {	// deactivate corresponding variable
		parent->ActivateVariable(buttonName, FALSE);
	}
	return TRUE;
}

LRESULT CClassifierDialog::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	EndDialog(IDOK);
	return 0;
}

LRESULT CClassifierDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}



Classifier::Classifier() :
	m_ClassifierDialog(this) {

	isTrained = false;
    isOnDisk = false;
    classifierType = 0;
	threshold = 0.5;
    filterImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
    applyImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
	guessMask = cvCreateImage(cvSize(GUESSMASK_WIDTH, GUESSMASK_HEIGHT), IPL_DEPTH_8U, 1);
    filterBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
    applyBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);

    // set the standard "friendly name"
    wcscpy(friendlyName, L"Generic Classifier");

    // create a directory to store this in
    WCHAR rootpath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, rootpath);
    int classifiernum = (int)time(0);
    wsprintf(directoryName, L"%s\\%s\\%s%d", rootpath, APP_CLASS, FILE_CLASSIFIER_PREFIX, classifiernum);

	// Initialize contour storage
	contourStorage = cvCreateMemStorage(0);

	// Create the default variables (all classifiers have these)
	outputData.AddVariable("Mask", guessMask);
	outputData.AddVariable("BoundingBoxes", &boundingBoxes, true);
	outputData.AddVariable("NumRegions", (int)0, false);
	outputData.AddVariable("TotalArea", (int)0, false);
	Point pt(0,0);
	outputData.AddVariable("Centroid", pt, false);
	outputData.AddVariable("Contours", (CvSeq*)NULL, false);
}

Classifier::Classifier(LPCWSTR pathname) :
	m_ClassifierDialog(this) {

	USES_CONVERSION;

	isTrained = true;
    isOnDisk = true;
    classifierType = 0;
	threshold = 0.5;

	filterImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
    applyImage = cvCreateImage(cvSize(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT), IPL_DEPTH_8U, 3);
	guessMask = cvCreateImage(cvSize(GUESSMASK_WIDTH, GUESSMASK_HEIGHT), IPL_DEPTH_8U, 1);
    filterBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);
    applyBitmap = new Bitmap(FILTERIMAGE_WIDTH, FILTERIMAGE_HEIGHT, PixelFormat24bppRGB);

	// save the directory name for later
	wcscpy(directoryName, pathname);

	// load the "friendly name"
	WCHAR filename[MAX_PATH];
	wcscpy(filename, pathname);
	wcscat(filename, FILE_FRIENDLY_NAME);
	FILE *namefile = fopen(W2A(filename), "r");
	fgetws(friendlyName, MAX_PATH, namefile);
	fclose(namefile);

	// load the threshold
	wcscpy(filename, pathname);
	wcscat(filename, FILE_THRESHOLD_NAME);
	FILE *threshfile = fopen(W2A(filename), "r");
	fread(&threshold, sizeof(float), 1, threshfile);
	fclose(threshfile);

	// load the filter sample image
    wcscpy(filename, pathname);
    wcscat(filename, FILE_DEMOIMAGE_NAME);
    IplImage *filterImageCopy = cvLoadImage(W2A(filename));
    cvCopy(filterImageCopy, filterImage);
    cvReleaseImage(&filterImageCopy);
    IplToBitmap(filterImage, filterBitmap);

	// Initialize contour storage
	contourStorage = cvCreateMemStorage(0);

	// Create the default variables (all classifiers have these)
	outputData.AddVariable("Mask", guessMask);
	outputData.AddVariable("BoundingBoxes", &boundingBoxes, true);
	outputData.AddVariable("NumRegions", (int)0, false);
	outputData.AddVariable("TotalArea", (int)0, false);
	Point pt(0,0);
	outputData.AddVariable("Centroid", pt, false);
	outputData.AddVariable("Contours", (CvSeq*)NULL, false);
}

Classifier::~Classifier() {
    cvReleaseImage(&filterImage);
    cvReleaseImage(&applyImage);
	cvReleaseImage(&guessMask);
	cvReleaseMemStorage(&contourStorage);
    delete filterBitmap;
    delete applyBitmap;
}


void Classifier::Save() {
	USES_CONVERSION;
	WCHAR filename[MAX_PATH];

	// make sure the directory exists 
    SHCreateDirectory(NULL, directoryName);

	// save the "friendly name"
	wcscpy(filename,directoryName);
	wcscat(filename, FILE_FRIENDLY_NAME);
	FILE *namefile = fopen(W2A(filename), "w");
	fputws(friendlyName, namefile);
	fclose(namefile);

	// save the threshold
	wcscpy(filename,directoryName);
	wcscat(filename, FILE_THRESHOLD_NAME);
	FILE *threshfile  = fopen(W2A(filename), "w");
	fwrite(&threshold, sizeof(float), 1, threshfile);
	fclose(threshfile);

	// Save the positive and negative training examples along with the classifier
	trainSet.Save(directoryName);

	// save the filter sample image
    wcscpy(filename, directoryName);
    wcscat(filename, FILE_DEMOIMAGE_NAME);
	cvSaveImage(W2A(filename), filterImage);

	isOnDisk = true;
}

void Classifier::Configure() {
	m_ClassifierDialog.DoModal();
}

void Classifier::DeleteFromDisk() {
    if (!isOnDisk) return;
    DeleteDirectory(directoryName, true);
    isOnDisk = false;
}

CvSeq* Classifier::GetMaskContours() {
    // reset the contour storage
    cvClearMemStorage(contourStorage);

    CvSeq* contours = NULL;
	cvFindContours(guessMask, contourStorage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	if (contours != NULL) {
        contours = cvApproxPoly(contours, sizeof(CvContour), contourStorage, CV_POLY_APPROX_DP, 3, 1 );
	}
	return contours;
}

Bitmap* Classifier::GetFilterImage() {
    return filterBitmap;
}
Bitmap* Classifier::GetApplyImage() {
    return applyBitmap;
}
LPWSTR Classifier::GetName() {
    return friendlyName;
}
void Classifier::SetName(LPCWSTR newName) {
    wcscpy(friendlyName, newName);
}

void Classifier::ActivateVariable(LPCWSTR varName, bool state) {
	USES_CONVERSION;
	string name = W2A(varName);
	outputData.SetVariableState(name, state);
}

void Classifier::UpdateStandardOutputData() {
	outputData.SetVariable("Mask", guessMask);
	CvSeq *contours = GetMaskContours();
	outputData.SetVariable("Contours", contours);

	// compute bounding boxes of mask contours, along with area and centroid, and count # of regions
	boundingBoxes.clear();
	Point centroid(0,0);
	int nRegions = 0;
	int totalArea = 0;
    if (contours != NULL){
        for (CvSeq *contour = contours; contour != NULL; contour = contour->h_next) {
            CvRect cvr = cvBoundingRect(contour, 1);
			totalArea += fabs(cvContourArea(contour));
			Rect r(cvr.x, cvr.y, cvr.width, cvr.height);
			boundingBoxes.push_back(r);
			centroid.X += (cvr.x+cvr.width/2);
			centroid.Y += (cvr.y+cvr.height/2);
			nRegions++;
        }
		if (nRegions > 0) {
			centroid.X /= nRegions;
			centroid.Y /= nRegions;
		}
	}
	outputData.SetVariable("BoundingBoxes", &boundingBoxes);
	outputData.SetVariable("NumRegions", nRegions);
	outputData.SetVariable("TotalArea", totalArea);
	outputData.SetVariable("Centroid", centroid);
}
