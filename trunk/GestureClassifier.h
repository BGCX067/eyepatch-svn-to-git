#pragma once
#include "Classifier.h"

class GestureClassifier : public Classifier {
public:
    GestureClassifier();
	GestureClassifier(LPCWSTR pathname);
    ~GestureClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);

    // run classifier on current entire motion track
    ClassifierOutputData ClassifyTrack(MotionTrack mt);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
	void UpdateTrajectoryImage();
	Recognizer rec;

    int nTemplates;
    int maxTemplateLength;
};
