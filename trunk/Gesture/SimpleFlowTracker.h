#pragma once

#include "../resource.h"

class SimpleFlowTracker
{
public:
	SimpleFlowTracker();
    ~SimpleFlowTracker(void);
    MotionTrack GetCurrentTrajectory();
	void ClearCurrentTrajectory();
	void StartTracking(IplImage *firstFrame);
	void StopTracking();
	void ProcessFrame(IplImage *frame);

	bool isInitialized;
    IplImage *outputFrame;

private:

	list<OneDollarPoint> trajectory;
	double currentX, currentY;

	int numInactiveFrames;

	// Images to store current and previous frame, in color and in grayscale
	IplImage *currentFrame, *grcurrentFrame, *grlastFrame;
	
	// Arrays to store detected features
	CvPoint2D32f currframe_features[200];
	CvPoint2D32f lastframe_features[200];
	char found_features[200];
	float feature_error[200];

	// Some arrays needed as workspace for the optical flow algorithm
	IplImage *eigimage, *tempimage, *pyramid1, *pyramid2;
};
