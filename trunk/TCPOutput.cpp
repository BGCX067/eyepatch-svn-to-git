#include "precomp.h"
#include "constants.h"

#include "OutputSink.h"
#include "TCPOutput.h"

TCPOutput::TCPOutput() :
    OutputSink() {

    SetName(L"XML over TCP on Port 8000");

    // Begin server listen thread
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerListenThread, (LPVOID)&server, 0, &threadID);
}

TCPOutput::~TCPOutput() {

    // End server listen thread
	TerminateThread(m_hThread, 0);
}

DWORD WINAPI TCPOutput::ServerListenThread(SocketServer* server) {
    while (true) {
        server->WaitForConnection();
        while (server->IsConnected()) {
            Sleep(100);
        }
    }
    return 1L;
}

void TCPOutput::ProcessInput(IplImage *image) {
}

void TCPOutput::ProcessOutput(IplImage *image, ClassifierOutputData data, char *filterName) {
    char buffer[TCP_OUTPUT_BUFFER_SIZE];
    char message[TCP_OUTPUT_BUFFER_SIZE];
	int ival;
	float fval;
	Point pt;
	vector<Rect> *bboxes;
	string sval;

    // If nobody is connected to the server, don't bother doing anything
    if (!server.IsConnected()) return;

    sprintf(buffer,"<FRAME ID=\"%s\">\r\n", filterName);

	int nVars = data.NumVariables();
	for (int i=0; i<nVars; i++) {
		ClassifierOutputVariable var = data.data[i];
		if (var.GetState() == true) {	// this variable is active
			switch(var.GetType()) {
				case CVAR_VOID:
				case CVAR_IMAGE:
					// can't do anything with these types
					break;
				case CVAR_INT:
					ival = var.GetIntData();
					sprintf(message,"<%s VAL=\"%d\" />\r\n", var.GetName().c_str(), ival);
					strcat(buffer, message);
					break;
				case CVAR_FLOAT:
					fval = var.GetFloatData();
					sprintf(message,"<%s VAL=\"%f\" />\r\n", var.GetName().c_str(), fval);
					strcat(buffer, message);
					break;
				case CVAR_POINT:
					pt = var.GetPointData();
					sprintf(message,"<%s X=\"%d\" Y=\"%d\" />\r\n", var.GetName().c_str(), ((int)pt.X), ((int)pt.Y));
					strcat(buffer, message);
					break;
				case CVAR_STRING:
					sval = var.GetStringData();
					sprintf(message,"<%s VAL=\"%s\" />\r\n", var.GetName().c_str(), sval.c_str());
					strcat(buffer, message);
					break;
				case CVAR_SEQ:
					// TODO: not yet implemented (how to format contour data as XML?)
					break;
				case CVAR_BBOXES:
					bboxes = var.GetBoundingBoxData();
					for (vector<Rect>::iterator box = bboxes->begin(); box != bboxes->end(); box++) {
						Rect r = (*box);
						sprintf(message,"<%s X=\"%d\" Y=\"%d\" WIDTH=\"%d\" HEIGHT=\"%d\" />\r\n",
							var.GetName().c_str(), (int)r.X, (int)r.Y, (int)r.Width, (int)r.Height);
						strcat(buffer, message);
					}
					break;
			}
		}
	}
    strcat(buffer,"</FRAME>\r\n\0");
    server.SendData(buffer);
}