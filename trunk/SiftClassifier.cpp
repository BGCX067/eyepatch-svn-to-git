#include "precomp.h"
#include "constants.h"

// Eyepatch includes
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "SiftClassifier.h"

SiftClassifier::SiftClassifier() :
	Classifier() {
    numSampleFeatures = 0;
    sampleCopy = NULL;
    sampleFeatures = NULL;

    // set the default "friendly name" and type
    wcscpy(friendlyName, L"SIFT Recognizer");
    classifierType = SIFT_FILTER;

    // append identifier to directory name
    wcscat(directoryName, FILE_SIFT_SUFFIX);
}

SiftClassifier::SiftClassifier(LPCWSTR pathname) :
	Classifier(pathname) {
	USES_CONVERSION;
    numSampleFeatures = 0;
    sampleCopy = NULL;
    sampleFeatures = NULL;

    WCHAR filename[MAX_PATH];
    wcscpy(filename, pathname);
    wcscat(filename, FILE_DATA_NAME);

    // load the features from the data file
    numSampleFeatures = import_features(W2A(filename), FEATURE_LOWE, &sampleFeatures);

	// load the filter sample image
    wcscpy(filename, pathname);
    wcscat(filename, FILE_SIFTIMAGE_NAME);
    sampleCopy = cvLoadImage(W2A(filename));
    sampleWidth = sampleCopy->width;
    sampleHeight = sampleCopy->height;

	// set the type
    classifierType = SIFT_FILTER;

    UpdateSiftImage();
}

SiftClassifier::~SiftClassifier() {
    if (sampleCopy) cvReleaseImage(&sampleCopy);
    if (numSampleFeatures > 0) free(sampleFeatures);
}

BOOL SiftClassifier::ContainsSufficientSamples(TrainingSet *sampleSet) {
    return (sampleSet->posSampleCount > 0);
}

void SiftClassifier::StartTraining(TrainingSet *sampleSet) {
	// Make a copy of the set used for training (we'll want to save it later)
	sampleSet->CopyTo(&trainSet);

    if (sampleCopy) cvReleaseImage(&sampleCopy);
    if (numSampleFeatures > 0) free(sampleFeatures);

    // TODO: call into trainingset class to do this instead of accessing samplemap
    for (map<UINT, TrainingSample*>::iterator i = sampleSet->sampleMap.begin(); i != sampleSet->sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample

            // store a copy of the sample image for later
            sampleCopy = cvCloneImage(sample->fullImageCopy);
            sampleWidth = sampleCopy->width;
            sampleHeight = sampleCopy->height;
            numSampleFeatures = sift_features(sample->fullImageCopy, &sampleFeatures);

            // for now just get features from first sample
            // TODO: collect feature sets from each sample?
            break;
		} else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
        }
    }

    if (isOnDisk) { // this classifier has been saved so we'll update the files
        Save();        
    }

    // update member variables
	isTrained = true;

    UpdateSiftImage();
}

ClassifierOutputData SiftClassifier::ClassifyFrame(IplImage *frame) {
	cvZero(guessMask);
	if (!isTrained) return outputData;
    if(!frame) return outputData;

    // copy current frame and sample image for demo image
    IplImage *frameCopy = cvCloneImage(frame);
    IplImage *featureImage = cvCloneImage(sampleCopy);
    IplImage *newMask = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
    cvZero(newMask);

    // get features in current frame
    struct feature *frameFeatures;
    int nFeatures = sift_features(frameCopy, &frameFeatures);

    if (nFeatures > 0) {

        // we'll use these points to find the bounding rectangle of the matching features
        CvPoint ptMin, ptMax;
        ptMax.x = 0;            
        ptMax.y = 0;
        ptMin.x = frameCopy->width;
        ptMin.y = frameCopy->height;

        struct kd_node* kd_root = kdtree_build(frameFeatures, nFeatures);
        struct feature** nbrs;
        numFeatureMatches = 0;
        for(int i=0; i<numSampleFeatures; i++)
        {
            struct feature *feat = sampleFeatures + i;
            sampleFeatures[i].fwd_match = NULL;
            int k = kdtree_bbf_knn(kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
            if( k == 2 ) {
                double d0 = descr_dist_sq(feat, nbrs[0]);
                double d1 = descr_dist_sq(feat, nbrs[1]);
                if(d0 < d1*NN_SQ_DIST_RATIO_THR) {
                    // the feature at ptSample in sample image corresponds to ptFrame in current frame
                    CvPoint ptSample = cvPoint(cvRound(feat->x), cvRound(feat->y) );
                    CvPoint ptFrame = cvPoint(cvRound(nbrs[0]->x), cvRound(nbrs[0]->y));
                    ptMin.x = min(ptMin.x, ptFrame.x);  ptMin.y = min(ptMin.y, ptFrame.y);
                    ptMax.x = max(ptMax.x, ptFrame.x);  ptMax.y = max(ptMax.y, ptFrame.y);

                    // draw feature in filter image
                    cvCircle(featureImage, ptSample, 2, colorSwatch[numFeatureMatches % COLOR_SWATCH_SIZE], 3, 8);

                    // draw feature in frame image
                    cvCircle(frameCopy, ptFrame, 2, colorSwatch[numFeatureMatches % COLOR_SWATCH_SIZE], 4, 8);

                    numFeatureMatches++;
                    sampleFeatures[i].fwd_match = nbrs[0];
                }
            }
            free( nbrs );
        }

        // As a starting point, compute the bounding box of matched features (in case we can't find transform)
        Rect objRect;
        objRect.X = ptMin.x;
        objRect.Y = ptMin.y;
        objRect.Width = ptMax.x - ptMin.x;
        objRect.Height = ptMax.y - ptMin.y;

		if (numFeatureMatches >= SIFT_MIN_RANSAC_FEATURES) {
            // try to use RANSAC algorithm to find transformation
            CvMat* H = ransac_xform(sampleFeatures, numSampleFeatures, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01, homog_xfer_err, 3.0, NULL, NULL );
            if (H != NULL) {
                IplImage* xformed;

                double pts[] = {0,0,sampleWidth,0,sampleWidth,sampleHeight,0,sampleHeight};
                CvMat foundRect = cvMat(1, 4, CV_64FC2, pts);
                cvPerspectiveTransform(&foundRect, &foundRect, H);

                cvLine(frameCopy, cvPoint(pts[0],pts[1]), cvPoint(pts[2],pts[3]), CV_RGB(255,255,255), 3);
                cvLine(frameCopy, cvPoint(pts[2],pts[3]), cvPoint(pts[4],pts[5]), CV_RGB(255,255,255), 3);
                cvLine(frameCopy, cvPoint(pts[4],pts[5]), cvPoint(pts[6],pts[7]), CV_RGB(255,255,255), 3);
                cvLine(frameCopy, cvPoint(pts[6],pts[7]), cvPoint(pts[0],pts[1]), CV_RGB(255,255,255), 3);
        
                objRect.X = min(min(pts[0],pts[2]),min(pts[4],pts[6]));
                objRect.Y = min(min(pts[1],pts[3]),min(pts[5],pts[7]));
                objRect.Width = max(max(pts[0],pts[2]),max(pts[4],pts[6])) - objRect.X;
                objRect.Height = max(max(pts[1],pts[3]),max(pts[5],pts[7])) - objRect.Y;

                cvReleaseMat( &H );
            }
        }

		int minMatches = 1 + threshold*6;
        if (numFeatureMatches > minMatches) { // we had enough matches to declare the object detected

            // draw object location guess in mask image
            cvRectangle(newMask, cvPoint(objRect.X, objRect.Y),
                cvPoint(objRect.X+objRect.Width, objRect.Y+objRect.Height),
                cvScalar(0xFF), CV_FILLED, 8); 
        }

        kdtree_release( kd_root );
        free(frameFeatures);
    }

    cvResize(featureImage, filterImage);
    IplToBitmap(filterImage, filterBitmap);

    cvResize(frameCopy, applyImage);
    IplToBitmap(applyImage, applyBitmap);

	// copy the final output mask
    cvResize(newMask, guessMask);

    cvReleaseImage(&frameCopy);
    cvReleaseImage(&featureImage);
	cvReleaseImage(&newMask);

	UpdateStandardOutputData();
	return outputData;
}

void SiftClassifier::UpdateSiftImage() {
    IplImage *featureImage = cvCloneImage(sampleCopy);
    draw_features(featureImage, sampleFeatures, numSampleFeatures);
    cvResize(featureImage, filterImage);
    cvReleaseImage(&featureImage);
    IplToBitmap(filterImage, filterBitmap);
}

void SiftClassifier::Save() {
    if (!isTrained) return;

	Classifier::Save();

    USES_CONVERSION;
    WCHAR filename[MAX_PATH];

	// save the feature data
    wcscpy(filename,directoryName);
    wcscat(filename, FILE_DATA_NAME);
    export_features(W2A(filename), sampleFeatures, numSampleFeatures);

	// save the SIFT source sample image
    wcscpy(filename, directoryName);
    wcscat(filename, FILE_SIFTIMAGE_NAME);
	cvSaveImage(W2A(filename), sampleCopy);
}
