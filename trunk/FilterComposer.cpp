#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "VideoLoader.h"
#include "VideoRecorder.h"
#include "BrightnessClassifier.h"
#include "ColorClassifier.h"
#include "ShapeClassifier.h"
#include "SiftClassifier.h"
#include "HaarClassifier.h"
#include "MotionClassifier.h"
#include "GestureClassifier.h"
#include "BackgroundSubtraction.h"
#include "TesseractClassifier.h"
#include "OutputSink.h"
#include "OSCOutput.h"
#include "TCPOutput.h"
#include "ClipboardOutput.h"
#include "StreamingVideoOutput.h"
#include "FilterLibrary.h"
#include "VideoRunner.h"
#include "FilterComposer.h"

CFilterComposer::CFilterComposer() :
    m_filterLibrary(this),
    m_videoRunner(this),
    labelFont(L"Verdana", 10),
	smallFont(L"Verdana", 8),
    whiteBrush(Color(255,255,255)),
    blackBrush(Color(0,0,0)),
    blackPen(Color(0,0,0),4),
    arrowPen(Color(100,255,100), 10) {
    arrowPen.SetEndCap(LineCapArrowAnchor);
}

CFilterComposer::~CFilterComposer() {
}

LRESULT CFilterComposer::OnPaint( UINT, WPARAM, LPARAM, BOOL& ) {
	PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);

    graphics->Clear(Color(240,240,240));

    graphics->FillRectangle(&whiteBrush, 10, 40, 320, 240);
    graphics->DrawRectangle(&blackPen, 10, 40, 320, 240);
    graphics->DrawString(L"INPUT VIDEO", 11, &labelFont, PointF(15,45), &blackBrush);
    if (m_videoRunner.processingVideo) {
        graphics->DrawImage(m_videoRunner.bmpInput, 10, 40, 320, 240);
	    graphics->DrawString(L"INPUT VIDEO", 11, &labelFont, PointF(15,45), &whiteBrush);
    }

    graphics->FillRectangle(&whiteBrush, 10, 420, 320, 240);
    graphics->DrawRectangle(&blackPen, 10, 420, 320, 240);
    graphics->DrawString(L"FILTERED VIDEO", 14, &labelFont, PointF(15,425), &blackBrush);
    if (m_videoRunner.processingVideo) {
        graphics->DrawImage(m_videoRunner.bmpOutput, 10, 420, 320, 240);
	    graphics->DrawString(L"FILTERED VIDEO", 14, &labelFont, PointF(15,425), &whiteBrush);
    }

	if (m_videoRunner.processingVideo) {
		// draw the blob tracking status image
		if (m_videoRunner.trackingGesture) {
			graphics->DrawImage(m_videoRunner.bmpGesture, 10, 290, 160, 120);
			graphics->DrawString(L"GESTURE INPUT", 13, &smallFont, PointF(15,295), &whiteBrush);
		}

		// draw the motion tracking status image
		if (m_videoRunner.trackingMotion) {
			graphics->DrawImage(m_videoRunner.bmpMotion, 170, 290, 160, 120);
			graphics->DrawString(L"MOTION INPUT", 12, &smallFont, PointF(175,295), &whiteBrush);
		}
	}

	BitBlt(hdc,FILTERLIBRARY_WIDTH,0,WINDOW_X-FILTERLIBRARY_WIDTH,WINDOW_Y,hdcmem,0,0,SRCCOPY);
    EndPaint(&ps);

    return 0;
}

LRESULT CFilterComposer::OnButtonDown( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

    return 0;
}

LRESULT CFilterComposer::OnMouseMove( UINT, WPARAM wParam, LPARAM lParam, BOOL& )
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

	return 0;
}

LRESULT CFilterComposer::OnButtonUp( UINT, WPARAM, LPARAM lParam, BOOL&)
{
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);

	return 0;
}

LRESULT CFilterComposer::OnCreate(UINT, WPARAM, LPARAM, BOOL& )
{
	HDC hdc = GetDC();
	hdcmem = CreateCompatibleDC(hdc);
	hbm = CreateCompatibleBitmap(hdc,WINDOW_X-FILTERLIBRARY_WIDTH,WINDOW_Y);
	SelectObject(hdcmem,hbm);
	ReleaseDC(hdc);
    graphics = new Graphics(hdcmem);
	graphics->SetSmoothingMode(SmoothingModeAntiAlias);
    graphics->Clear(Color(240,240,240));

    // Create the filter library dialog
    m_filterLibrary.Create(m_hWnd, WS_CHILD | WS_VISIBLE);
    m_filterLibrary.MoveWindow(0, 0, FILTERLIBRARY_WIDTH, WINDOW_Y);
    m_filterLibrary.ShowWindow(TRUE);

	// Set the starting combine mode to "LIST"
	m_videoRunner.filterCombineMode = IDC_COMBINE_LIST;
	m_filterLibrary.CheckRadioButton(IDC_COMBINE_LIST,IDC_COMBINE_CASCADE,IDC_COMBINE_LIST);
	return 0;
}

LRESULT CFilterComposer::OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) {

	// Stop running (if we are currently processing video)
	m_videoRunner.StopProcessing();

	// Free the loaded classifiers and outputsinks
	ClearActiveClassifiers();
    ClearStandardClassifiers();
    ClearCustomClassifiers();
    ClearOutputs();
    
    delete graphics;
	DeleteDC(hdcmem);
	DeleteObject(hbm);

    m_filterLibrary.DestroyWindow();

    PostQuitMessage( 0 );
	return 0;
}

LRESULT CFilterComposer::OnAddCustomFilter( UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND listView;
    switch(wParam) {
        case COLOR_FILTER:
        case SHAPE_FILTER:
        case SIFT_FILTER:
        case BRIGHTNESS_FILTER:
        case ADABOOST_FILTER:
        case MOTION_FILTER:
        case GESTURE_FILTER:
            bool newlyAdded = m_videoRunner.AddActiveFilter((Classifier*)lParam);
			if (newlyAdded) {
				listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_FILTER_LIST);
				m_filterLibrary.AddFilter(listView, (Classifier*)lParam);
				::InvalidateRect(listView, NULL, FALSE);
			}
            break;
    }
    return 0;
}

LRESULT CFilterComposer::OnAddStandardFilter( UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND listView;
    bool newlyAdded  = m_videoRunner.AddActiveFilter((Classifier*)lParam);
	if (newlyAdded) {
		listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_FILTER_LIST);
		m_filterLibrary.AddFilter(listView, (Classifier*)lParam);
		::InvalidateRect(listView, NULL, FALSE);
	}
    return 0;
}

LRESULT CFilterComposer::OnAddOutputSink( UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND listView;
	bool newlyAdded = m_videoRunner.AddActiveOutput((OutputSink*)lParam);
	if (newlyAdded) {
		listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_OUTPUT_LIST);
		m_filterLibrary.AddOutput(listView, (OutputSink*)lParam);
		::InvalidateRect(listView, NULL, FALSE);
	}
    return 0;
}


LRESULT CFilterComposer::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HMENU hMenu;
    switch(wParam) {
        case IDC_RUNLIVE:
            hMenu = ::GetMenu(this->GetParent());
            if (!m_videoRunner.processingVideo) {
				m_videoRunner.ResetActiveFilterRunningStates();
                m_videoRunner.StartProcessing(true);
                if (m_videoRunner.processingVideo) {
                    m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).EnableWindow(FALSE);
                    m_filterLibrary.GetDlgItem(IDC_RUNLIVE).SetWindowText(L"Stop Running");

                    // disable the menu
                    EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED);
                    ::DrawMenuBar(GetParent());
                }
            } else {
                m_videoRunner.StopProcessing();
                m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).EnableWindow(TRUE);
                m_filterLibrary.GetDlgItem(IDC_RUNLIVE).SetWindowText(L"Run on Live Video!");

                // reenable the menu
                EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED);
                ::DrawMenuBar(GetParent());
            }
            break;
		case IDC_RUNRECORDED:
            hMenu = ::GetMenu(this->GetParent());
            if (!m_videoRunner.processingVideo) {
				m_videoRunner.ResetActiveFilterRunningStates();
                m_videoRunner.StartProcessing(false);
                if (m_videoRunner.processingVideo) {
                    m_filterLibrary.GetDlgItem(IDC_RUNLIVE).EnableWindow(FALSE);
                    m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).SetWindowText(L"Stop Running");

                    // disable the menu
                    EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
                    EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_GRAYED);
                    ::DrawMenuBar(GetParent());
                }
            } else {
                m_videoRunner.StopProcessing();
                m_filterLibrary.GetDlgItem(IDC_RUNLIVE).EnableWindow(TRUE);
                m_filterLibrary.GetDlgItem(IDC_RUNRECORDED).SetWindowText(L"Run on Recorded Video...");

                // reenable the menu
                EnableMenuItem(hMenu, 0, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
                EnableMenuItem(hMenu, 2, MF_BYPOSITION | MF_ENABLED);
                ::DrawMenuBar(GetParent());
            }
			break;
        case IDC_RESET:
			ResetState();
            break;
		case IDC_COMBINE_LIST:
		case IDC_COMBINE_AND:
		case IDC_COMBINE_OR:
		case IDC_COMBINE_CASCADE:
			m_videoRunner.filterCombineMode = wParam;
			break;
        default:
            break;
    }
    return 0;
}

void CFilterComposer::ResetState() {
    ClearActiveClassifiers();
    ClearActiveOutputs();
	// Set the combine mode back to "LIST"
	m_videoRunner.filterCombineMode = IDC_COMBINE_LIST;
	m_filterLibrary.CheckRadioButton(IDC_COMBINE_LIST,IDC_COMBINE_CASCADE,IDC_COMBINE_LIST);
}

void CFilterComposer::LoadCustomClassifier(LPWSTR pathname) {

    Classifier *newclassifier = NULL;

    if (wcsstr(pathname, FILE_BRIGHTNESS_SUFFIX) != NULL) {
        newclassifier = new BrightnessClassifier(pathname);
    } else if (wcsstr(pathname, FILE_COLOR_SUFFIX) != NULL) { 
        newclassifier = new ColorClassifier(pathname);
    } else if (wcsstr(pathname, FILE_GESTURE_SUFFIX) != NULL) { 
        newclassifier = new GestureClassifier(pathname);
    } else if (wcsstr(pathname, FILE_HAAR_SUFFIX) != NULL) { 
        newclassifier = new HaarClassifier(pathname);
    } else if (wcsstr(pathname, FILE_MOTION_SUFFIX) != NULL) { 
        newclassifier = new MotionClassifier(pathname);
    } else if (wcsstr(pathname, FILE_SHAPE_SUFFIX) != NULL) { 
        newclassifier = new ShapeClassifier(pathname);
    } else if (wcsstr(pathname, FILE_SIFT_SUFFIX) != NULL) { 
        newclassifier = new SiftClassifier(pathname);
    }

    if (newclassifier != NULL) {
        customClassifiers.push_back(newclassifier);
        HWND listView = m_filterLibrary.GetDlgItem(IDC_MY_FILTER_LIST);
        m_filterLibrary.AddFilter(listView, newclassifier);
    }
}

void CFilterComposer::ClearCustomClassifiers() {
    for (list<Classifier*>::iterator i = customClassifiers.begin(); i!= customClassifiers.end(); i++) {
        delete (*i);
    }
    HWND listView = m_filterLibrary.GetDlgItem(IDC_MY_FILTER_LIST);
    m_filterLibrary.ClearFilters(listView);
    customClassifiers.clear();
}

void CFilterComposer::ClearActiveClassifiers() {
	m_videoRunner.ResetActiveFilterRunningStates();
    m_videoRunner.ClearActiveFilters();
    HWND listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_FILTER_LIST);
    m_filterLibrary.ClearFilters(listView);
}

void CFilterComposer::LoadStandardClassifiers() {
    HWND listView = m_filterLibrary.GetDlgItem(IDC_STD_FILTER_LIST);
	Classifier *c;

    // Background subtraction
    c = new BackgroundSubtraction();
    standardClassifiers.push_back(c);
    m_filterLibrary.AddFilter(listView, c);

    // Tesseract OCR
    c = new TesseractClassifier();
    standardClassifiers.push_back(c);
    m_filterLibrary.AddFilter(listView, c);
}

void CFilterComposer::ClearStandardClassifiers() {
    for (list<Classifier*>::iterator i = standardClassifiers.begin(); i!= standardClassifiers.end(); i++) {
        delete (*i);
    }
    HWND listView = m_filterLibrary.GetDlgItem(IDC_STD_FILTER_LIST);
    m_filterLibrary.ClearFilters(listView);
    standardClassifiers.clear();
}

void CFilterComposer::LoadOutputs() {
    HWND listView = m_filterLibrary.GetDlgItem(IDC_OUTPUT_LIST);

    // OSC over UDP output
    OutputSink *osc = new OSCOutput();
    outputSinks.push_back(osc);
    m_filterLibrary.AddOutput(listView, osc);

    // XML over TCP output
    OutputSink *tcp = new TCPOutput();
    outputSinks.push_back(tcp);
    m_filterLibrary.AddOutput(listView, tcp);

	// Clipboard output
    OutputSink *clipboard = new ClipboardOutput();
    outputSinks.push_back(clipboard);
    m_filterLibrary.AddOutput(listView, clipboard);

	// video streaming output
	OutputSink *streamvideo = new StreamingVideoOutput();
    outputSinks.push_back(streamvideo);
    m_filterLibrary.AddOutput(listView, streamvideo);

}

void CFilterComposer::ClearOutputs() {
    for (list<OutputSink*>::iterator i = outputSinks.begin(); i!= outputSinks.end(); i++) {
        delete (*i);
    }
    HWND listView = m_filterLibrary.GetDlgItem(IDC_MY_FILTER_LIST);
    m_filterLibrary.ClearFilters(listView);
    customClassifiers.clear();
}

void CFilterComposer::ClearActiveOutputs() {
    HWND listView = m_filterLibrary.GetDlgItem(IDC_ACTIVE_OUTPUT_LIST);
    m_filterLibrary.ClearOutputs(listView);
    m_videoRunner.ClearActiveOutputs();
}

