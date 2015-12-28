#pragma once
#include "SimpleFlowTracker.h"

class CFilterComposer;

class CVideoRunner
{
public: 
	CVideoRunner(CWindow *caller);
	~CVideoRunner();

    static DWORD WINAPI ThreadCallback(CVideoRunner*);
    void StartProcessing(bool isLive);
    void StopProcessing();
	void ApplyFilterChain();
	void ProcessFrame();
	ClassifierOutputData GetStandardOutputData();

    bool AddActiveFilter(Classifier*);
    void ClearActiveFilters();
	void ResetActiveFilterRunningStates();

    bool AddActiveOutput(OutputSink *o);
    void ClearActiveOutputs();

	BOOL LoadRecordedVideo(HWND hwndOwner, CvCapture** capture);

	int videoX, videoY;
    int fps;
    long nFrames, framesAvailable;
    bool processingVideo, runningLive;
    IplImage *copyFrame, *outputFrame, *outputAccImage, *contourMask;
	Bitmap *bmpInput, *bmpOutput, *bmpMotion, *bmpGesture;

    //  keep track of the number of active filters that require motion or blob tracking
    int trackingMotion;
    int trackingGesture;

	// we can combine filters as LIST, AND, OR, or CASCADE
	int filterCombineMode;

	// for the bounding boxes output from a combination of filters
	vector<Rect> boundingBoxes;

private:
    CvCapture *videoCapture;
    IplImage *currentFrame, *guessMask, *combineMask, *combineMaskOutput, *motionHistory;
    IplImage* motionBuf[MOTION_NUM_IMAGES];

    // functions that may be called by ProcessFrame (if motion/gesture filters are active)
    void ProcessMotionFrame();
    void ProcessGestureFrame();

    // for keeping track of position within circular motion history buffer
    int last;

	// for tracking optical flow for gesture tracking
	SimpleFlowTracker m_flowTracker;

    // list of classifiers to apply to live video stream
    list<Classifier*> activeClassifiers;

    // list of outputs to which we will send video data
    list<OutputSink*> activeOutputs;

	// memory storage for contours of combine mask
	CvMemStorage *contourStorage;

	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
    CWindow *parent;
};
