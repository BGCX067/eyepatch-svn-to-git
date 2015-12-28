#pragma once
#include "../resource.h"

class FlowTracker;

class CFlowTrackerDialog : public CDialogImpl<CFlowTrackerDialog> {
public:
	CFlowTrackerDialog(FlowTracker*);
	~CFlowTrackerDialog();

    enum { IDD = IDD_FLOWTRACKER_DIALOG };
    BEGIN_MSG_MAP(CFlowTrackerDialog)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void ConvertFrames();

private:
	CRect videoRect;
	Rect drawRect;
	DWORD threadID;
	HANDLE m_hMutex;
	HANDLE m_hThread;
	static DWORD WINAPI ThreadCallback(CFlowTrackerDialog*);
	FlowTracker *parent;
};


class FlowTracker
{
public:
	FlowTracker();
    ~FlowTracker(void);
    void LearnTrajectories(CvCapture* pCap);
    MotionTrack GetTrajectoryInRange(long startFrame, long endFrame);
    MotionTrack GetTrajectoryAtFrame(long frameNum);
    void ProcessFrame();

    bool isTrained;

protected:
    IplImage *copyFrame;
    Bitmap *bmpVideo;
    CvCapture *videoCapture;

private:
    MotionTrack trajectory;
	double currentX, currentY;

	int numInactiveFrames;

	friend class CFlowTrackerDialog;
	CFlowTrackerDialog m_FlowTrackerDialog;

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
