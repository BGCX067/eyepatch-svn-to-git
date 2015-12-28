#pragma once

class CVideoLoader;

class CVideoLoaderDialog : public CDialogImpl<CVideoLoaderDialog> {
public:
	CVideoLoaderDialog(CVideoLoader*);
	~CVideoLoaderDialog();

    enum { IDD = IDD_VIDEOLOADER_DIALOG };
    BEGIN_MSG_MAP(CVideoLoaderDialog)
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
	static DWORD WINAPI ThreadCallback(CVideoLoaderDialog*);
	CVideoLoader *parent;
};

class CVideoLoader
{
public: 
	CVideoLoader();
	~CVideoLoader();
	BOOL OpenVideoFile(HWND);
	BOOL OpenVideoFile(HWND,LPCWSTR);
    void LoadFrame(long);
    IplImage* GetMotionHistory();
    void LearnTrajectories();
	void ConvertFrame();
    Bitmap* GetMaskedBitmap();
    MotionTrack GetTrajectoryInRange(long startFrame, long endFrame);
    MotionTrack GetTrajectoryAtCurrentFrame();

    long nFrames;
	int videoX, videoY;
    int currentFrameNumber;
    BOOL videoLoaded;
    IplImage *copyFrame;
    Bitmap *bmpVideo;

    // stores the current mask associated with the filter chain
    IplImage *guessMask;

private:

    // for producing masked image frame
    IplImage *maskedFrame;
    Bitmap *bmpMasked;

    CvCapture *videoCapture;
	CvVideoWriter *videoWriter;
	
    IplImage *currentFrame;
    IplImage *motionHistory;

    FlowTracker *m_flowTracker;

	friend class CVideoLoaderDialog;
	CVideoLoaderDialog m_hVideoLoaderDialog;
};
