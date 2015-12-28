#pragma once
#include "ClassifierOutputData.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "../resource.h"

class Classifier;

class CClassifierDialog : public CDialogImpl<CClassifierDialog> {
public:
	CClassifierDialog(Classifier* c);
	~CClassifierDialog();

    enum { IDD = IDD_CLASSIFIER_DIALOG };
    BEGIN_MSG_MAP(CClassifierDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnButtonClicked)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnButtonClicked(UINT uMsg, WPARAM wParam, HWND hwndButton, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	Classifier *parent;
};



class Classifier
{
public:

	Classifier();
	Classifier(LPCWSTR pathname);
	virtual ~Classifier();

	virtual void StartTraining(TrainingSet*) = 0;
    virtual BOOL ContainsSufficientSamples(TrainingSet*) = 0;
	virtual ClassifierOutputData ClassifyFrame(IplImage*) = 0;
	virtual void ResetRunningState() = 0;

	virtual void Save();
	void Configure();
    void DeleteFromDisk();
	CvSeq* GetMaskContours();
    Bitmap* GetFilterImage();
    Bitmap* GetApplyImage();
    LPWSTR GetName();
    void SetName(LPCWSTR newName);
	void ActivateVariable(LPCWSTR varName, bool state);
	void UpdateStandardOutputData();

	ClassifierOutputData outputData;
	bool isTrained;
    bool isOnDisk;
    int classifierType;
	float threshold;

protected:
    Bitmap *filterBitmap, *applyBitmap;
    IplImage *filterImage, *applyImage, *guessMask;
	CvMemStorage *contourStorage;
	vector<Rect> boundingBoxes;
	TrainingSet trainSet;	// samples last used to train classifier

	friend class CClassifierDialog;
	CClassifierDialog m_ClassifierDialog;

    WCHAR friendlyName[MAX_PATH];
    WCHAR directoryName[MAX_PATH];
};
