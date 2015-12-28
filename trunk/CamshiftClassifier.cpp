#include "precomp.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "CamshiftClassifier.h"

CamshiftClassifier::CamshiftClassifier() :
	Classifier() {
	image = NULL;
	hsv = NULL;
	hue = NULL;
	mask = NULL;
	backproject = NULL;
	histimg = NULL;
	hist = NULL;

	hdims = 16;
	hranges_arr[0] = 0;	hranges_arr[1] = 180;
	hranges = hranges_arr;

	// allocate histogram
	hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
}

CamshiftClassifier::~CamshiftClassifier() {
	// free histogram
	cvReleaseHist(&hist);
}

void CamshiftClassifier::StartTraining(TrainingSet *sampleSet) {

	// clear out the histogram
	cvClearHist(hist);

	// TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == 0) { // positive sample

			// allocate image buffers
			hsv = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 3 );
			hue = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );
			mask = cvCreateImage( cvGetSize(sample->fullImageCopy), 8, 1 );

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

		} else if (sample->iGroupId == 1) { // negative sample
			// TODO: we could potentially subtract this from histogram
        }
    }

	// create histogram image
	histimg = cvCreateImage( cvSize(320,200), 8, 3 );
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

	// update member variables
	nPosSamples = sampleSet->posSampleCount;
	nNegSamples = sampleSet->negSampleCount;
	isTrained = true;
}

void CamshiftClassifier::ClassifyFrame(IplImage *frame, list<Rect>* objList) {
    if (!isTrained) return;
    if(!frame) return;

    image = cvCreateImage( cvGetSize(frame), 8, 3 );
    hsv = cvCreateImage( cvGetSize(frame), 8, 3 );
    hue = cvCreateImage( cvGetSize(frame), 8, 1 );
    mask = cvCreateImage( cvGetSize(frame), 8, 1 );
	backproject = cvCreateImage( cvGetSize(frame), 8, 1 );

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

	// find contours in backprojection image
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = NULL;
    cvFindContours( backproject, storage, &contours, sizeof(CvContour),
                    CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

	// Loop over the found contours
	objList->clear();
	for (; contours != NULL; contours = contours->h_next)
	{
        double contourArea = fabs(cvContourArea(contours));
		if ((contourArea > COLOR_MIN_AREA) && (contourArea < COLOR_MAX_AREA)) {
			Rect objRect;
			CvRect rect = cvBoundingRect(contours);
			objRect.X = rect.x;
			objRect.Y = rect.y;
			objRect.Width = rect.width;
			objRect.Height = rect.height;
			objList->push_back(objRect);

            // draw contour in demo image
            cvDrawContours(image, contours, CV_RGB(0,255,255), CV_RGB(0,255,255), 0, 2, 8);
        }
	}

    // update bitmap demo image
    cvResize(image, applyImage);
    IplToBitmap(applyImage, applyBitmap);

	cvReleaseMemStorage(&storage);

	cvReleaseImage(&image);
	cvReleaseImage(&hsv);
	cvReleaseImage(&hue);
	cvReleaseImage(&mask);
	cvReleaseImage(&backproject);
}
