#pragma once
#include "Classifier.h"
#include "MotionClassifier.h"
#include "GestureClassifier.h"
#include "VideoLoader.h"

class ClassifierTester : public CDialogImpl<ClassifierTester> 
{
public:
	ClassifierTester();
	~ClassifierTester();

    enum { IDD = IDD_QUICKTEST_DIALOG };
    BEGIN_MSG_MAP(CClassifierDialog)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnPaint( UINT, WPARAM, LPARAM, BOOL& );
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void TestClassifierOnVideo(Classifier *c, CVideoLoader *vl, int recognizerMode);

private:
	IplImage *quickTestImage;
    Bitmap *quickTestBitmap;
	void RunClassifierOnCurrentFrame(Classifier *c, CVideoLoader *vl, int recognizerMode);
};
