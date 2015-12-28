#include "precomp.h"
#include "constants.h"

// Eyepatch includes
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "HaarClassifier.h"

HaarClassifierDialog::HaarClassifierDialog(HaarClassifier *p) {
	parent = p;
	m_hThread = NULL;
}

HaarClassifierDialog::~HaarClassifierDialog() {
}

LRESULT HaarClassifierDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    CenterWindow();
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)this, 0, &threadID);
	return TRUE;    // let the system set the focus
}

LRESULT HaarClassifierDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	TerminateThread(m_hThread, 0);
	parent->isTrained = false;
	if (parent->nStagesCompleted >= MIN_HAAR_STAGES) {
		parent->cascade = cvLoadHaarClassifierCascade(parent->classifierName, cvSize(HAAR_SAMPLE_X, HAAR_SAMPLE_Y));
		if (parent->cascade != NULL) {
			parent->isTrained = true;
            if (parent->isOnDisk) { // this classifier has been saved so we'll update the files
                parent->Save();        
            }
		}
	}
	return 0;
}

DWORD WINAPI HaarClassifierDialog::ThreadCallback(HaarClassifierDialog* instance) {
	instance->Train();
	return 1L;
}

void HaarClassifierDialog::Train() {
    cvCreateCascadeClassifier(parent->classifierPathname,  parent->vecFilename, parent->negFilename, 
        parent->nPosSamples, parent->nNegSamples, parent->nStages,
		0, 2, .99, .5, .95, 3, 0, 1, HAAR_SAMPLE_X, HAAR_SAMPLE_Y, 3, 0,
		GetDlgItem(IDC_HAAR_PROGRESS), &(parent->nStagesCompleted));
	::EndDialog(m_hWnd, IDOK);
}


HaarClassifier::HaarClassifier() :
	Classifier(),
	m_progressDlg(this) {
    cascade = NULL;
    nStages = START_HAAR_STAGES;
    storage = cvCreateMemStorage(0);
	nPosSamples = 0;
	nNegSamples = 0;
	nStagesCompleted = 0;

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"Adaboost Recognizer");
    classifierType = ADABOOST_FILTER;        

    // append identifier to directory name
    wcscat(directoryName, FILE_HAAR_SUFFIX);
}

HaarClassifier::HaarClassifier(LPCWSTR pathname) :
	Classifier(pathname),
	m_progressDlg(this) {

	USES_CONVERSION;
    cascade = NULL;
    nStages = START_HAAR_STAGES;
    storage = cvCreateMemStorage(0);
	nPosSamples = 0;
	nNegSamples = 0;
	nStagesCompleted = 0;

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_CASCADE_NAME);

    // load the cascade from the data file
	cascade = cvLoadHaarClassifierCascade(W2A(filename), cvSize(HAAR_SAMPLE_X, HAAR_SAMPLE_Y));
	if (cascade != NULL) {
		isTrained = true;
		isOnDisk = true;
	}

	// set the type
    classifierType = ADABOOST_FILTER;
}

HaarClassifier::~HaarClassifier() {
    cvReleaseMemStorage(&storage);
    if (isTrained) cvReleaseHaarClassifierCascade(&cascade);
}

BOOL HaarClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return ((sampleSet->posSampleCount > 3) && (sampleSet->negSampleCount > 3));
}

void HaarClassifier::PrepareData(TrainingSet *sampleSet) {

    int gridSize = (int) ceil(sqrt((double)sampleSet->posSampleCount));
    int gridX = 0;
    int gridY = 0;
    int gridSampleW = FILTERIMAGE_WIDTH / gridSize;
    int gridSampleH = FILTERIMAGE_HEIGHT / gridSize;
    cvZero(filterImage);

    char tempPathname[MAX_PATH];
    char imageFilename[MAX_PATH];

    GetTempPathA(MAX_PATH, tempPathname);
    sprintf_s(vecFilename, "%spossamples.vec", tempPathname);
    sprintf_s(negFilename, "%snegsamples.dat", tempPathname);
    int classifiernum = (int)time(0);
    sprintf_s(classifierPathname, "%sclassifier%d/", tempPathname, classifiernum);
    sprintf_s(classifierName, "%sclassifier%d", tempPathname, classifiernum);

    icvMkDir(vecFilename);
    icvMkDir(negFilename);
    FILE *vec = fopen(vecFilename, "wb");
    FILE *neglist = fopen(negFilename,"w");
    int imgNum=0;

    icvWriteVecHeader(vec, sampleSet->posSampleCount, HAAR_SAMPLE_X, HAAR_SAMPLE_Y);

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

            // create a scaled-down, grayscale version of the sample
            IplImage *sampleCopyColor = cvCreateImage(cvSize(HAAR_SAMPLE_X, HAAR_SAMPLE_Y), IPL_DEPTH_8U, 3);
            IplImage *sampleCopyGrayscale = cvCreateImage(cvSize(HAAR_SAMPLE_X, HAAR_SAMPLE_Y), IPL_DEPTH_8U, 1);
            if (sample->fullImageCopy->width >= HAAR_SAMPLE_X && sample->fullImageCopy->height >= HAAR_SAMPLE_Y) {
                cvResize(sample->fullImageCopy, sampleCopyColor, CV_INTER_AREA);
            } else { 
                cvResize(sample->fullImageCopy, sampleCopyColor, CV_INTER_LINEAR);
            }
            cvCvtColor(sampleCopyColor, sampleCopyGrayscale, CV_BGR2GRAY);

            // draw the grayscale version into the filter demo image
            cvCvtColor(sampleCopyGrayscale, sampleCopyColor, CV_GRAY2BGR);
            CvMat *filterImageSubRect = cvCreateMat(gridSampleW, gridSampleH, CV_8UC1);
            cvGetSubRect(filterImage, filterImageSubRect, cvRect(gridX*gridSampleW,gridY*gridSampleH,gridSampleW,gridSampleH));
            cvResize(sampleCopyColor, filterImageSubRect);
            cvReleaseMat(&filterImageSubRect);
            cvReleaseImage(&sampleCopyColor);

            // add the grayscale image to vector of positive samples
            icvWriteVecSample(vec, sampleCopyGrayscale);

            gridX++;
            if (gridX >= gridSize) {
                gridX = 0;
                gridY++;
            }

        } else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
            sprintf_s(imageFilename, "%sneg%d.jpg", tempPathname, imgNum);
            CvSize negImageSize = cvSize(sample->fullImageCopy->width, sample->fullImageCopy->height);

            if ((negImageSize.width < 2*HAAR_SAMPLE_X) || (negImageSize.height < 2*HAAR_SAMPLE_Y)) {
                negImageSize.width = max(negImageSize.width, 2*HAAR_SAMPLE_X);
                negImageSize.height = max(negImageSize.height, 2*HAAR_SAMPLE_Y);

                IplImage *negImageCopy = cvCreateImage(negImageSize, IPL_DEPTH_8U, 3);
                cvResize(sample->fullImageCopy, negImageCopy);
                cvSaveImage(imageFilename, negImageCopy);
                cvReleaseImage(&negImageCopy);
            } else {
                cvSaveImage(imageFilename, sample->fullImageCopy);
            }

            fprintf(neglist, "neg%d.jpg\n", imgNum);
            imgNum++;
        }
    }
    fclose(vec);
    fclose(neglist);
	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;

    // update demo image
    IplToBitmap(filterImage, filterBitmap);
}

void HaarClassifier::StartTraining(TrainingSet* sampleSet) {
	// Make a copy of the set used for training (we'll want to save it later)
	sampleSet->CopyTo(&trainSet);

	PrepareData(sampleSet);
	m_progressDlg.DoModal();
}

ClassifierOutputData HaarClassifier::ClassifyFrame(IplImage *frame) {
	cvZero(guessMask);
	if (!isTrained) return outputData;
    if (!cascade) return outputData;

    IplImage *newMask = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    cvZero(newMask);

    // Clear the memory storage we used before
    cvClearMemStorage( storage );

    // There can be more than one object in an image, so we create a growable sequence of objects
    // Detect the objects and store them in the sequence
    CvSeq* objects = cvHaarDetectObjects(frame, cascade, storage,
                                         1.1, (int)(1+threshold*4), CV_HAAR_DO_CANNY_PRUNING,
                                         cvSize(HAAR_SAMPLE_X, HAAR_SAMPLE_Y));

    IplImage *frameCopy = cvCreateImage(cvSize(frame->width,frame->height), IPL_DEPTH_8U, 3);
    cvCopy(frame, frameCopy);

    int objNum = 0;
    // Loop over the found objects
    for(int i = 0; i < (objects ? objects->total : 0); i++ )
    {
        CvRect* r = (CvRect*)cvGetSeqElem(objects, i);
        cvRectangle(frameCopy, cvPoint(r->x,r->y), cvPoint(r->x+r->width,r->y+r->height), colorSwatch[objNum], 2, 8);
        objNum = (objNum + 1) % COLOR_SWATCH_SIZE;

        // draw rectangle in mask image
        cvRectangle(newMask, cvPoint(r->x, r->y), cvPoint(r->x+r->width, r->y+r->height), cvScalar(0xFF), CV_FILLED, 8);
    }

	// copy the final output mask
    cvResize(newMask, guessMask);

    cvResize(frameCopy, applyImage);
    IplToBitmap(applyImage, applyBitmap);
	cvReleaseImage(&newMask);
	cvReleaseImage(&frameCopy);

	UpdateStandardOutputData();
	return outputData;
}

void HaarClassifier::Save() {
    if (!isTrained) return;

	Classifier::Save();

    USES_CONVERSION;
    WCHAR filename[MAX_PATH];

	// save the cascade data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_CASCADE_NAME);
	cvSave(W2A(filename), cascade, 0, 0, cvAttrList(0,0));
}
