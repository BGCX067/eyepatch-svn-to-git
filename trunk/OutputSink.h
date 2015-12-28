#pragma once
#include "ClassifierOutputData.h"

class OutputSink {
public:
    OutputSink() { }
    virtual ~OutputSink() { }

    LPWSTR GetName() {
        return friendlyName;
    }
    void SetName(LPCWSTR newName) {
        wcscpy(friendlyName, newName);
    }

    virtual void ProcessInput(IplImage* image) = 0;
    virtual void ProcessOutput(IplImage* image, ClassifierOutputData data, char* filterName) = 0;

	virtual void StartRunning() = 0;
	virtual void StopRunning() = 0;

private:
    WCHAR friendlyName[MAX_PATH];
};
