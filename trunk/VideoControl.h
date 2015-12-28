#pragma once

class CVideoControl : public CDialogImpl<CVideoControl> {
public:
   enum { IDD = IDD_VIDEOCONTROL_DIALOG };

    BEGIN_MSG_MAP(CVideoControl)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_HSCROLL, OnTrack)
        MESSAGE_HANDLER(WM_ENABLE, OnEnable)
    END_MSG_MAP()

    CVideoControl(CWindow *parent);
    ~CVideoControl();
    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnTrack(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnCommand(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnEnable(UINT, WPARAM, LPARAM, BOOL&);
    void EnableSelectionRange(BOOL enabled);

private:
    CWindow *parent;

    HANDLE hiMarkin, hiMarkout, hiFrameleft, hiFrameright;
};
