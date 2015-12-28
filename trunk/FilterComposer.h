#pragma once

typedef CWinTraits<WS_CHILD|WS_VISIBLE,0> CFilterComposerTraits;

class CFilterComposer: public CWindowImpl<CFilterComposer, CWindow, CFilterComposerTraits>
{
public:
    CFilterComposer();
    ~CFilterComposer();
    LRESULT OnPaint( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnButtonUp( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnButtonDown( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnMouseMove( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnCommand(UINT, WPARAM, LPARAM, BOOL& );

    LRESULT OnAddCustomFilter(UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnAddStandardFilter(UINT, WPARAM, LPARAM, BOOL& );
    LRESULT OnAddOutputSink(UINT, WPARAM, LPARAM, BOOL& );

    void LoadCustomClassifier(LPWSTR pathname);
    void ClearCustomClassifiers();

    void LoadStandardClassifiers();
    void ClearStandardClassifiers();
    void ClearActiveClassifiers();

    void LoadOutputs();
    void ClearOutputs();
    void ClearActiveOutputs();

	void ResetState();

    static CWndClassInfo& GetWndClassInfo()
    {
        static CWndClassInfo wc =
        {
            { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, 
            StartWindowProc,
            0, 0, NULL, NULL, NULL, (HBRUSH)(WHITE_BRUSH), NULL, 
            FILTER_CREATE_CLASS, LoadIcon(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDI_EYEPATCH)) },
            NULL, NULL, IDC_ARROW, TRUE, 0, FILTER_CREATE_CLASS
        };
        return wc;
    }

    BEGIN_MSG_MAP(CFilterComposer)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnButtonUp)
        MESSAGE_HANDLER(WM_RBUTTONUP, OnButtonUp)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnButtonDown)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_ADD_CUSTOM_FILTER, OnAddCustomFilter)
        MESSAGE_HANDLER(WM_ADD_STD_FILTER, OnAddStandardFilter)
        MESSAGE_HANDLER(WM_ADD_OUTPUT_SINK, OnAddOutputSink)
    END_MSG_MAP()

private:
	HDC hdcmem;
	HBITMAP hbm;
	Graphics *graphics;

    Pen blackPen, arrowPen;
    SolidBrush whiteBrush, blackBrush;
    Font labelFont, smallFont;

    CVideoRunner m_videoRunner;
    CFilterLibrary m_filterLibrary;
    list<Classifier*> customClassifiers;
    list<Classifier*> standardClassifiers;
    list<OutputSink*> outputSinks;
};
