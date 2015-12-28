#pragma once

class CVideoRecorder;

class CVideoRecorderDialog : public CDialogImpl<CVideoRecorderDialog> {
public:
	CVideoRecorderDialog(CVideoRecorder*);
	~CVideoRecorderDialog();

    enum { IDD = IDD_VIDEORECORDER_DIALOG };
    BEGIN_MSG_MAP(CVideoRecorderDialog)
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
	static DWORD WINAPI ThreadCallback(CVideoRecorderDialog*);
	CVideoRecorder *parent;
};

class CVideoRecorder
{
public: 
	CVideoRecorder();
	~CVideoRecorder();
	BOOL RecordVideoFile(HWND);
	void ConvertFrame();
    void GrabNextFrame();

    WCHAR szFileName[MAX_PATH];
	int videoX, videoY;
    int fps;
    int nFrames;
    bool recordingVideo;
    IplImage *copyFrame;
    Bitmap *bmpVideo;

private:
    CvCapture *videoCapture;
	CvVideoWriter *videoWriter;
	
    IplImage *currentFrame;

	friend class CVideoRecorderDialog;
	CVideoRecorderDialog m_hVideoRecorderDialog;
};
