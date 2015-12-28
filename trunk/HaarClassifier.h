#pragma once
#include "Classifier.h"

class HaarClassifier;

class HaarClassifierDialog : public CSimpleDialog<IDD_HAAR_DIALOG> {
public:
	HaarClassifierDialog(HaarClassifier*);
	~HaarClassifierDialog();

	BEGIN_MSG_MAP(CListDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CSimpleDialog<IDD_HAAR_DIALOG>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	HaarClassifier *parent;
	static DWORD WINAPI ThreadCallback(HaarClassifierDialog*);
	void Train();
	DWORD threadID;
	HANDLE m_hThread;

};

class HaarClassifier : public Classifier {
public:
    HaarClassifier();
	HaarClassifier(LPCWSTR pathname);
	~HaarClassifier();

    BOOL ContainsSufficientSamples(TrainingSet*);
	void StartTraining(TrainingSet*);
	ClassifierOutputData ClassifyFrame(IplImage*);
    void Save();
	void ResetRunningState() {}		// This classifier doesn't have store any new state info while running live

	int nStages, nStagesCompleted;

private:
	void PrepareData(TrainingSet*);

	CvHaarClassifierCascade* cascade;
    CvMemStorage* storage;

    int nPosSamples, nNegSamples;

    char vecFilename[MAX_PATH];
    char negFilename[MAX_PATH];
    char classifierPathname[MAX_PATH];
    char classifierName[MAX_PATH];

	friend class HaarClassifierDialog;
	HaarClassifierDialog m_progressDlg;

};
