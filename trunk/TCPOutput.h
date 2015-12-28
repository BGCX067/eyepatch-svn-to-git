#pragma once
#include "OutputSink.h"

// Socket server includes
#include "../SocketServer/SocketServer.h"

class TCPOutput : public OutputSink {

public:
    TCPOutput();
    ~TCPOutput();
	void ProcessInput(IplImage* image);
    void ProcessOutput(IplImage* image, ClassifierOutputData data, char* filterName);
	void StartRunning() {}	// the socket server is always running
	void StopRunning() {}   // so we don't need these functions

private:
    SocketServer server;
    HANDLE m_hThread;
	DWORD threadID;
	static DWORD WINAPI ServerListenThread(SocketServer*);
};
