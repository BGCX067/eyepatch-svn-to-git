#pragma once
#include "Classifier.h"

class TesseractClassifier : public Classifier {
public:
    TesseractClassifier();
    TesseractClassifier(LPCWSTR pathname);
    ~TesseractClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
    void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState();

private:
	TessDllAPI api;
};
