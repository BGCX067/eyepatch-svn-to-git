#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "TesseractClassifier.h"

TesseractClassifier::TesseractClassifier() :
	api("eng"),
	Classifier() {

    // set the "friendly name" and type
    wcscpy(friendlyName, L"Tesseract Text Recognition");
    classifierType = FILTER_BUILTIN;

    isTrained = false;
	isOnDisk = false;

	// Create the custom output variables for this classifier
	outputData.AddVariable("Text", "");

}

TesseractClassifier::TesseractClassifier(LPCWSTR pathname) :
	Classifier() {

    // We will never load standard filters from disk, so this should never be called
    assert(FALSE);
}

TesseractClassifier::~TesseractClassifier() {
}

BOOL TesseractClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {

    // Standard filters don't need training samples
    assert(FALSE);
    return true;
}

void TesseractClassifier::StartTraining(TrainingSet *sampleSet) {

    // Standard filters don't use training (this should never be called)
    assert(FALSE);
}

ClassifierOutputData TesseractClassifier::ClassifyFrame(IplImage *frame) {
	cvZero(guessMask);
    if(!frame) return outputData;

    IplImage *frameGray = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    IplImage *newMask = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    cvZero(newMask);

	cvCvtColor(frame, frameGray, CV_BGR2GRAY);
	cvThreshold(frameGray, frameGray, 0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);

	api.BeginPageUpright(frameGray->width, frameGray->height, (unsigned char*)frameGray->imageData, frameGray->depth);
	ETEXT_DESC* output = api.Recognize_all_Words();

	char buffer[TESSERACT_MAX_CHARS];
	int len = 0;

	for (int i = 0; i < output->count; i++) {
		const EANYCODE_CHAR* ch = &output->text[i];
		for (int b = 0; b < ch->blanks; ++b) {
			buffer[len] = ' ';
			len++;
		}
		if (ch->char_code <= 0x7f) {
			buffer[len] = ch->char_code;
			len++;
		}
		cvRectangle(newMask, cvPoint(ch->left, ch->top), cvPoint(ch->right, ch->bottom), cvScalar(0xFF), CV_FILLED);
	}
	buffer[len] = '\0';
	outputData.SetVariable("Text", buffer);

	// copy the final output mask
	cvResize(newMask, guessMask);

	UpdateStandardOutputData();

	cvReleaseImage(&newMask);

	return outputData;
}


void TesseractClassifier::Save() {
    
    // We can't save built-in classifiers, so this should never be called
    assert(FALSE);
}

void TesseractClassifier::ResetRunningState() {

}
