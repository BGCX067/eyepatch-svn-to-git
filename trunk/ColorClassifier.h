#pragma once
#include "Classifier.h"

class ColorClassifier : public Classifier {
public:
    ColorClassifier();
    ColorClassifier(LPCWSTR pathname);
    ~ColorClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
    void UpdateHistogramImage();

    CvHistogram *hist;
	int hdims;
};
