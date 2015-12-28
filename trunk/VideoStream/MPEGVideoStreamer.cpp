#include "precomp.h"
#include "MPEGVideoStreamer.h"

DWORD WINAPI MPEGVideoStreamer::ThreadCallback(LPVOID img) {
	IplImage *frame = (IplImage*) img;
	ULONG startTime = GetTickCount();
	ULONG framesSent = 0;

	CvVideoWriter *writer = cvCreateStreamingVideoWriter("rtp://127.0.0.1:9000?localport=9001", CV_FOURCC('M','P','G','2'), 24, cvSize(176, 144), 1);
	while(1) {
		ULONG timeElapsed = GetTickCount()-startTime;
		ULONG targetFramesSent = (ULONG)(24.0*timeElapsed)/1000.0;
		if (framesSent < targetFramesSent) {
			cvWriteStreamingFrame(writer, frame);
			framesSent++;
		}
	}
	cvReleaseVideoWriter(&writer);
	cvReleaseImage(&frame);
}

MPEGVideoStreamer::MPEGVideoStreamer() {
	isStreaming = false;
	frame = cvCreateImage(cvSize(176,144), 8, 3);
	cvZero(frame);
}

MPEGVideoStreamer::~MPEGVideoStreamer() {
	cvReleaseImage(&frame);
}

void MPEGVideoStreamer::StartStreaming() {
	if (isStreaming) return;
	isStreaming = true;
	m_hThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)ThreadCallback, (LPVOID)frame, 0, &threadID);
}

void MPEGVideoStreamer::UpdateFrame(IplImage *newFrame) {
	cvResize(newFrame, frame);
}

void MPEGVideoStreamer::StopStreaming() {
	if (!isStreaming) return;
	TerminateThread(m_hThread, 0);
	isStreaming = false;
}

bool MPEGVideoStreamer::IsStreaming() {
	return isStreaming;
}