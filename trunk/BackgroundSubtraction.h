#pragma once
#include "Classifier.h"

class BackgroundSubtraction : public Classifier {
public:
    BackgroundSubtraction();
    BackgroundSubtraction(LPCWSTR pathname);
    ~BackgroundSubtraction();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState();

private:
    long frameNum;
    CvBGStatModel* bgmodel;
	IplImage *smallFrameCopy, *fgMaskSmall;
};
