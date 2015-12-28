#pragma once

typedef CWinTraits<WS_VISIBLE|WS_SYSMENU|WS_CAPTION,0> EyepatchTraits;

class CEyepatch: public CWindowImpl<CEyepatch,CWindow,EyepatchTraits>
{
public:
    CEyepatch();
    ~CEyepatch();
	HINSTANCE m_hInstance;

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& );
	LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL& );
	LRESULT OnCommand( UINT, WPARAM, LPARAM, BOOL& );
    void LoadSampleFromFile();
    void LoadCreateModeClassifiers();
    void LoadComposeModeClassifiers();
	void DisplayVersionInfo();

	static CWndClassInfo& GetWndClassInfo()
	{
		static CWndClassInfo wc =
		{
			{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, 
			StartWindowProc,
			0, 0, NULL, NULL, NULL, (HBRUSH)(WHITE_BRUSH), NULL, 
            APP_CLASS, LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDI_EYEPATCH)) },
			NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
		};
		return wc;
	}

	BEGIN_MSG_MAP(CEyepatch)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    ALT_MSG_MAP(1)  // video markup
    END_MSG_MAP()

private:
    CVideoMarkup m_videoMarkup;
    CFilterComposer m_filterComposer;
    EyepatchMode m_mode;
    HMENU hMenu;
};
