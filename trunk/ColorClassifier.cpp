#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "ColorClassifier.h"

ColorClassifier::ColorClassifier() :
	Classifier() {

	// allocate histogram
	hdims = 16;
	float hranges_arr[2];
	hranges_arr[0] = 0;	hranges_arr[1] = 180;
	float* hranges = hranges_arr;
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"Color Recognizer");
    classifierType = COLOR_FILTER;        
    
    // append identifier to directory name
    wcscat(directoryName, FILE_COLOR_SUFFIX);   
}

ColorClassifier::ColorClassifier(LPCWSTR pathname) :
	Classifier(pathname) {

    USES_CONVERSION;

	// allocate histogram
	hdims = 16;
	float hranges_arr[2];
	hranges_arr[0] = 0;	hranges_arr[1] = 180;
	float* hranges = hranges_arr;
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_DATA_NAME);

    // load the data from the histogram file
    FILE *datafile = fopen(W2A(filename), "rb");
    for(int i = 0; i < hdims; i++) {
        float val;
        fread(&val, sizeof(float), 1, datafile);
		cvSetReal1D(hist->bins,i,val);
    }
    fclose(datafile);

	// set the type
	classifierType = COLOR_FILTER;

    UpdateHistogramImage();
}

ColorClassifier::~ColorClassifier() {
	// free histogram
	cvReleaseHist(&hist);
}

BOOL ColorClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->posSampleCount > 0);
}

void ColorClassifier::StartTraining(TrainingSet *sampleSet) {
	// Make a copy of the set used for training (we'll want to save it later)
	sampleSet->CopyTo(&trainSet);

	// clear out the histogram
	cvClearHist(hist);

	// TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

			// allocate image buffers
			IplImage *hsv = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 3 );
			IplImage *hue = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );
			IplImage *mask = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );

			// convert to hsv space
			cvCvtColor(sample->fullImageCopy, hsv, CV_BGR2HSV);

			// clip max and min range and split out hue channel
			cvInRangeS(hsv, cvScalar(0,COLOR_SMIN,COLOR_VMIN,0),cvScalar(180,256,COLOR_VMAX,0), mask);
			cvSplit(hsv, hue, 0, 0, 0);

			// accumulate into hue histogram
			cvCalcHist(&hue, hist, 1, mask);

			// free image buffers
			cvReleaseImage(&hsv);
			cvReleaseImage(&hue);
			cvReleaseImage(&mask);

		} else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
			// TODO: we could potentially subtract this from histogram
        }
    }

    UpdateHistogramImage();

    if (isOnDisk) { // this classifier has been saved so we'll update the files
        Save();        
    }

	// update member variables
	isTrained = true;
}

ClassifierOutputData ColorClassifier::ClassifyFrame(IplImage *frame) {
	cvZero(guessMask);
	if (!isTrained) return outputData;
    if(!frame) return outputData;

    IplImage *image = cvCreateImage( cvGetSize(frame), 8, 3 );
    IplImage *hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
    IplImage *hue = cvCreateImage( cvGetSize(frame), 8, 1 );
    IplImage *mask = cvCreateImage( cvGetSize(frame), 8, 1 );
	IplImage *backproject = cvCreateImage( cvGetSize(frame), 8, 1 );
    IplImage *newMask = cvCreateImage( cvGetSize(frame), 8, 1 );
    cvZero(newMask);

    cvCopy( frame, image, 0 );
    cvCvtColor( image, hsv, CV_BGR2HSV );

	// create mask to clip out pixels outside of specified range
	cvInRangeS(hsv, cvScalar(0,COLOR_SMIN,COLOR_VMIN,0), cvScalar(180,256,COLOR_VMAX,0), mask);
    cvSplit(hsv, hue, 0, 0, 0 );

	// create backprojection image and clip with mask
    cvCalcBackProject(&hue, backproject, hist);
    cvAnd(backproject, mask, backproject, 0);

    // copy back projection into demo image
    cvCvtColor(backproject, image, CV_GRAY2BGR);

    // create contour storage
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;

	// threhold the backprojection image
	cvThreshold(backproject, backproject, threshold*255, 255, CV_THRESH_BINARY);

    // close the backprojection image
    IplConvKernel *circElem = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_ELLIPSE);        
    cvMorphologyEx(backproject, backproject, 0, circElem, CV_MOP_CLOSE, 1);  
	cvReleaseStructuringElement(&circElem);

    // find contours in backprojection image
    cvFindContours( backproject, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// Loop over the found contours
	for (; contours != NULL; contours = contours->h_next)
	{
        double contourArea = fabs(cvContourArea(contours));
		if ((contourArea > COLOR_MIN_AREA) && (contourArea < COLOR_MAX_AREA)) {

            // draw contour in new mask image
            cvDrawContours(newMask, contours, cvScalar(0xFF), cvScalar(0xFF), 0, CV_FILLED, 8);

            // draw contour in demo image
            cvDrawContours(image, contours, CV_RGB(0,255,255), CV_RGB(0,255,255), 0, 2, 8);
        }
	}

	// copy the final output mask
    cvResize(newMask, guessMask);

    // update bitmap demo image
    cvResize(image, applyImage);
    IplToBitmap(applyImage, applyBitmap);

	cvReleaseMemStorage(&storage);

	cvReleaseImage(&image);
	cvReleaseImage(&hsv);
	cvReleaseImage(&hue);
	cvReleaseImage(&mask);
	cvReleaseImage(&backproject);
	cvReleaseImage(&newMask);

	UpdateStandardOutputData();
	return outputData;
}

void ColorClassifier::UpdateHistogramImage() {

    // create histogram image
	IplImage *histimg = cvCreateImage( cvSize(320,200), 8, 3 );
	float max_val = 0.f;
	cvGetMinMaxHistValue( hist, 0, &max_val, 0, 0 );
	cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );
	cvZero( histimg );
	int bin_w = histimg->width / hdims;
	for(int i = 0; i < hdims; i++)
	{
		int val = cvRound( cvGetReal1D(hist->bins,i)*histimg->height/255 );
		CvScalar color = hsv2rgb(i*180.f/hdims);
		cvRectangle( histimg, cvPoint(i*bin_w,histimg->height),
			cvPoint((i+1)*bin_w,histimg->height - val),
			color, -1, 8, 0 );
	}
    cvResize(histimg, filterImage);
    IplToBitmap(filterImage, filterBitmap);
    cvReleaseImage(&histimg);
}


void ColorClassifier::Save() {
    if (!isTrained) return;

	Classifier::Save();

    USES_CONVERSION;
    WCHAR filename[MAX_PATH];

    // save the histogram data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_DATA_NAME);
    FILE *datafile = fopen(W2A(filename), "wb");
	for(int i = 0; i < hdims; i++) {
		float val = cvGetReal1D(hist->bins,i);
        fwrite(&val, sizeof(float), 1, datafile);
	}
    fclose(datafile);
}
