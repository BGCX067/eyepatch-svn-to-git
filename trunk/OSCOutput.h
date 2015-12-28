#pragma once
#include "OutputSink.h"

// OSC Includes
#include "../OSCPack/osc/OscOutboundPacketStream.h"
#include "../OSCPack/ip/UdpSocket.h"

class OSCOutput : public OutputSink {
public:
    OSCOutput();
    ~OSCOutput();

	void ProcessInput(IplImage* image);
	void ProcessOutput(IplImage* image, ClassifierOutputData data, char* filterName);
	void StartRunning() {}	// nothing is sent when there's no data,
	void StopRunning() {}   // so we don't need these functions

private:
    UdpTransmitSocket *transmitSocket;
};
