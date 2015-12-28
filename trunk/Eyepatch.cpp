#include "precomp.h"
#include "constants.h"
#include "resource.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "VideoLoader.h"
#include "VideoRecorder.h"
#include "Classifier.h"
#include "OutputSink.h"
#include "FilterSelect.h"
#include "VideoControl.h"
#include "ClassifierTester.h"
#include "VideoMarkup.h"
#include "VideoRunner.h"
#include "FilterLibrary.h"
#include "FilterComposer.h"
#include "Eyepatch.h"

CEyepatch::CEyepatch() {
    m_mode = EYEPATCHMODE_CREATEFILTERS;
}


CEyepatch::~CEyepatch() {
}

LRESULT CEyepatch::OnCommand( UINT, WPARAM wParam, LPARAM lParam, BOOL& ) {
    CvCapture *capture;
    if (HIWORD(wParam) == 0) { // this command came from a menu
        switch (LOWORD(wParam)) {
            case ID_FILE_OPENVIDEO:
                m_videoMarkup.OpenVideoFile();
                break;
            case ID_FILE_RECORDVIDEO:
                m_videoMarkup.RecordVideoFile();
                break;
            case ID_FILE_OPENSAMPLE:
                LoadSampleFromFile();
                break;
            case ID_FILE_EMPTYTRASH:
                m_videoMarkup.EmptyTrash();
                break;
            case ID_SETTINGS_CAPTURESOURCE:
                capture = cvCreateCameraCapture(0);
                cvSetCaptureProperty(capture, CV_CAP_PROP_DIALOG_SOURCE, 0 );
                cvReleaseCapture(&capture);
                break;
            case ID_SETTINGS_CAPTUREFORMAT:
                capture = cvCreateCameraCapture(0);
                cvSetCaptureProperty(capture, CV_CAP_PROP_DIALOG_FORMAT, 0 );
                cvReleaseCapture(&capture);
                break;
            case ID_SETTINGS_VERSIONINFO:
				this->DisplayVersionInfo();
				break;
            case ID_FILE_EXIT:
                PostMessage(WM_CLOSE, 0, 0);
                break;
            case ID_MODE_RUNRECOGNIZERS:
                // Reload the custom classifiers, in case they have changed on disk
                LoadComposeModeClassifiers();

                // Update menu for filter composer mode
                CheckMenuItem(hMenu, ID_MODE_RUNRECOGNIZERS, MF_CHECKED);
                CheckMenuItem(hMenu, ID_MODE_CREATERECOGNIZERS, MF_UNCHECKED);
                EnableMenuItem(hMenu, ID_FILE_OPENVIDEO, MF_GRAYED);
                EnableMenuItem(hMenu, ID_FILE_OPENSAMPLE, MF_GRAYED);
                EnableMenuItem(hMenu, ID_FILE_RECORDVIDEO, MF_GRAYED);
                EnableMenuItem(hMenu, ID_FILE_EMPTYTRASH, MF_GRAYED);
                m_videoMarkup.EnableWindow(FALSE);
                m_videoMarkup.SetWindowPos(HWND_BOTTOM, 0, 0, WINDOW_X, WINDOW_Y, SWP_HIDEWINDOW);
                m_filterComposer.SetWindowPos(HWND_TOP, 0, 0, WINDOW_X, WINDOW_Y, SWP_SHOWWINDOW);
                m_filterComposer.EnableWindow(TRUE);
                m_mode = EYEPATCHMODE_RUNFILTERS;
                break;
            case ID_MODE_CREATERECOGNIZERS:
                CheckMenuItem(hMenu, ID_MODE_RUNRECOGNIZERS, MF_UNCHECKED);
                CheckMenuItem(hMenu, ID_MODE_CREATERECOGNIZERS, MF_CHECKED);
                EnableMenuItem(hMenu, ID_FILE_OPENVIDEO, MF_ENABLED);
                EnableMenuItem(hMenu, ID_FILE_OPENSAMPLE, MF_ENABLED);
                EnableMenuItem(hMenu, ID_FILE_RECORDVIDEO, MF_ENABLED);
                EnableMenuItem(hMenu, ID_FILE_EMPTYTRASH, MF_ENABLED);
				m_filterComposer.ResetState();
                m_filterComposer.EnableWindow(FALSE);
                m_filterComposer.SetWindowPos(HWND_BOTTOM, 0, 0, WINDOW_X, WINDOW_Y, SWP_HIDEWINDOW);
                m_videoMarkup.SetWindowPos(HWND_TOP, 0, 0, WINDOW_X, WINDOW_Y, SWP_SHOWWINDOW);
                m_videoMarkup.EnableWindow(TRUE);
                m_mode = EYEPATCHMODE_CREATEFILTERS;
                break;
        }
    }
    return 0;
}

LRESULT CEyepatch::OnCreate(UINT, WPARAM, LPARAM, BOOL& )
{
    m_videoMarkup.Create(m_hWnd, CRect(0,0,WINDOW_X,WINDOW_Y), FILTER_CREATE_CLASS, WS_CHILD | WS_VISIBLE);
    m_filterComposer.Create(m_hWnd, CRect(0,0,WINDOW_X,WINDOW_Y), FILTER_COMPOSE_CLASS, WS_CHILD);

    // Create the menu
    hMenu = LoadMenu(_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDR_MENU));
    SetMenu(hMenu);

    // Load the classifiers from disk
    LoadCreateModeClassifiers();

    // Load the outputs (these never change, so we don't need to reload when switching modes
    m_filterComposer.LoadOutputs();

    // Load the standard filters (these don't change either)
    m_filterComposer.LoadStandardClassifiers();

    return 0;
}


LRESULT CEyepatch::OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) {

    m_filterComposer.DestroyWindow();
    m_videoMarkup.DestroyWindow();

    DestroyMenu(hMenu);
    PostQuitMessage( 0 );
	return 0;
}

void CEyepatch::DisplayVersionInfo() {
	USES_CONVERSION;
	WCHAR versionInfo[1024];
    const char* opencv_libraries = 0;
    const char* addon_modules = 0;
    cvGetModuleInfo( 0, &opencv_libraries, &addon_modules );
	wsprintf(versionInfo, L"Eyepatch Version: %s\n", EYEPATCH_VERSION);
	wcscat(versionInfo, L"\nOpenCV Libraries: "); 
	wcscat(versionInfo, A2W(opencv_libraries)); 
	wcscat(versionInfo, L"\nAdd-On Modules: "); 
	wcscat(versionInfo, A2W(addon_modules));

	MSGBOXPARAMS mbp;
	mbp.hwndOwner = this->m_hWnd;
	mbp.hInstance = this->m_hInstance;
	mbp.dwStyle = MB_USERICON;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_EYEPATCH);
	mbp.dwContextHelpId = NULL;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = NULL;
	mbp.lpszCaption = L"Eyepatch Version Information";
	mbp.lpszText = versionInfo;

	::MessageBoxIndirect(&mbp);
}

void CEyepatch::LoadSampleFromFile() {
    USES_CONVERSION;
    WCHAR szFileNameList[MAX_PATH*16] = L"";
    WCHAR szPathName[MAX_PATH] = L"";
    WCHAR szFileName[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.dib;*.jpeg;*.pbm;*.ppm;*.pgm;*.sr;*.ras;*.tiff;*.tif\0";
    ofn.lpstrFile = szFileNameList;
    ofn.nMaxFile = MAX_PATH*16;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"jpg";

    if(!GetOpenFileName(&ofn)) {
        return;
    }

    WCHAR *strPtr = szFileNameList;
    wcscpy(szPathName, strPtr);
    strPtr += (wcslen(strPtr) + 1);

    if (wcslen(strPtr) == 0) { // only one filename selected
        m_videoMarkup.OpenSampleFile(W2A(szPathName));
    } else { // step through all the filenames
        while(wcslen(strPtr) != 0) {
            wcscpy(szFileName, szPathName);
            wcscat(szFileName, L"\\" );
            wcscat(szFileName, strPtr);
            m_videoMarkup.OpenSampleFile(W2A(szFileName));
            strPtr += (wcslen(strPtr) + 1);
        }
    }
}

void CEyepatch::LoadCreateModeClassifiers() {

    WCHAR rootpath[MAX_PATH];
    WCHAR searchpath[MAX_PATH];
    WCHAR fullpath[MAX_PATH];

    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, rootpath);
    PathAppend(rootpath, APP_CLASS);

    wcscpy(searchpath,rootpath);
    PathAppend(searchpath, L"*.*");
   
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;

    if ((hFind = FindFirstFile(searchpath, &win32fd)) == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (win32fd.cFileName[0] != '.' && 
           (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
           !(win32fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

               wcscpy(fullpath, rootpath);
               PathAppend(fullpath, win32fd.cFileName);
               m_videoMarkup.LoadClassifier(fullpath);
        }
    } while(FindNextFile(hFind, &win32fd) != 0);
    FindClose(hFind);
}

void CEyepatch::LoadComposeModeClassifiers() {

    WCHAR rootpath[MAX_PATH];
    WCHAR searchpath[MAX_PATH];
    WCHAR fullpath[MAX_PATH];

    // Clear the old list of custom classifiers
    m_filterComposer.ClearCustomClassifiers();

    // Clear the old list of active classifiers
    m_filterComposer.ClearActiveClassifiers();

    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, rootpath);
    PathAppend(rootpath, APP_CLASS);

    wcscpy(searchpath,rootpath);
    PathAppend(searchpath, L"*.*");
   
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;

    if ((hFind = FindFirstFile(searchpath, &win32fd)) == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (win32fd.cFileName[0] != '.' && 
           (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
           !(win32fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

               wcscpy(fullpath, rootpath);
               PathAppend(fullpath, win32fd.cFileName);
               m_filterComposer.LoadCustomClassifier(fullpath);
        }
    } while(FindNextFile(hFind, &win32fd) != 0);
    FindClose(hFind);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	MSG msg;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CEyepatch *wnd = new CEyepatch();
	wnd->m_hInstance = hInstance;

	wnd->Create(NULL, CRect(0,0,WINDOW_X,WINDOW_Y), APP_CLASS,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE);
	while( GetMessage( &msg, NULL, 0, 0 ) ){
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	delete wnd;

	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}
