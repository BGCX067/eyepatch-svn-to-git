#pragma once
#include "OutputSink.h"
#include "MPEGVideoStreamer.h"

class StreamingVideoOutput : public OutputSink {
public:
    StreamingVideoOutput();
    ~StreamingVideoOutput();
    void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, ClassifierOutputData data, char* filterName);
	void StartRunning();
	void StopRunning();
private:
	MPEGVideoStreamer streamer;
};
