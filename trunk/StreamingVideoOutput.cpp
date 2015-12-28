#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "StreamingVideoOutput.h"

StreamingVideoOutput::StreamingVideoOutput() :
    OutputSink() {
    SetName(L"Stream MPEG-2 Video over RTP to Port 9000");
}

StreamingVideoOutput::~StreamingVideoOutput() {
	streamer.StopStreaming();
}

void StreamingVideoOutput::ProcessInput(IplImage *image) {
	if (!streamer.IsStreaming()) {
		streamer.StartStreaming();
	}
	streamer.UpdateFrame(image);
}

void StreamingVideoOutput::ProcessOutput(IplImage *image, ClassifierOutputData data, char *filterName) {
}

void StreamingVideoOutput::StartRunning() {
//	streamer.StartStreaming();
}

void StreamingVideoOutput::StopRunning() {
	streamer.StopStreaming();
}