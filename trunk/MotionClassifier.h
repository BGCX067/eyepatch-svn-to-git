#pragma once
#include "Classifier.h"

class MotionClassifier : public Classifier {
public:
    MotionClassifier();
    MotionClassifier(LPCWSTR pathname);
    ~MotionClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    ClassifierOutputData ClassifyMotion(IplImage*, double);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
    list<double> motionAngles;
};
