#pragma once
#include "Classifier.h"

class ShapeClassifier : public Classifier {
public:
    ShapeClassifier();
    ShapeClassifier(LPCWSTR pathname);
    ~ShapeClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

private:
    void UpdateContourImage();

    CvMemStorage *templateStorage;
    CvSeq *templateContours;
};
