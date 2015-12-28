#pragma once
#include "OutputSink.h"

class ClipboardOutput : public OutputSink {
public:
    ClipboardOutput();
    ~ClipboardOutput();
    void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, ClassifierOutputData data, char* filterName);
	void StartRunning() {}	// nothing is sent when there's no data,
	void StopRunning() {}   // so we don't need these functions
};
