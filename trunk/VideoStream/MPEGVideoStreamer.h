#pragma once

class MPEGVideoStreamer
{
public:
	MPEGVideoStreamer();
	~MPEGVideoStreamer();

	void StartStreaming();
	void UpdateFrame(IplImage *newFrame);
	void StopStreaming();
	bool IsStreaming();

private:
	static DWORD WINAPI ThreadCallback(LPVOID);

	IplImage *frame;
	DWORD threadID;
	HANDLE m_hThread;
	bool isStreaming;
};
