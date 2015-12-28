#pragma once
#include "Classifier.h"

class BrightnessClassifier : public Classifier {
public:
    BrightnessClassifier();
    BrightnessClassifier(LPCWSTR pathname);
    ~BrightnessClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
    void UpdateHistogramImage();

	CvHistogram *hist;
    int hdims;
	float avg_level;  // the average brightness value (we threshold on this)
};
