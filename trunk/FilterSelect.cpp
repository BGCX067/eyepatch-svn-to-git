#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "FilterSelect.h"

CFilterSelect::CFilterSelect(CWindow *caller) {
    parent = caller;
}

CFilterSelect::~CFilterSelect(void) {
}

LRESULT CFilterSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	// Populate the combo box with the different classifier types
    HWND hwndCombo = GetDlgItem(IDC_FILTER_COMBO);
	for (int i=0; i<NUM_FILTERS; i++) {
		ComboBox_InsertString(hwndCombo, -1, filterNames[i]);
	}
	
	// Set the range of the threshold slider from 0 to 100, and intial value to 50
    SendDlgItemMessage(IDC_FILTER_THRESHOLD, TBM_SETRANGEMIN, FALSE, 0);
    SendDlgItemMessage(IDC_FILTER_THRESHOLD, TBM_SETRANGEMAX, FALSE, 100);
    SendDlgItemMessage(IDC_FILTER_THRESHOLD, TBM_SETPOS, TRUE, 50);
    return 0;
}

LRESULT CFilterSelect::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    parent->SendMessage(uMsg, wParam, lParam);
    return 0;
}

LRESULT CFilterSelect::OnTrack(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	long sliderPosition = (long) SendDlgItemMessage(IDC_FILTER_THRESHOLD, TBM_GETPOS, 0, 0);
	bool isDragging = (LOWORD(wParam) != SB_ENDSCROLL);
	parent->SendMessage(WM_SET_THRESHOLD, ((WPARAM)sliderPosition), isDragging);
	return 0;
}

LRESULT CFilterSelect::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    GetDlgItem(IDC_TRAINBUTTON).EnableWindow(wParam);
    GetDlgItem(IDC_SHOWBUTTON).EnableWindow(wParam);
    GetDlgItem(IDC_QUICKTEST).EnableWindow(wParam);
    GetDlgItem(IDC_SAVEFILTER).EnableWindow(wParam);
    GetDlgItem(IDC_FILTER_LIST).EnableWindow(wParam);
    GetDlgItem(IDC_FILTER_THRESHOLD).EnableWindow(wParam);
    GetDlgItem(IDC_FILTER_COMBO).EnableWindow(wParam);
    return 1;
}

LRESULT CFilterSelect::OnTextCallback(int idCtrl, LPNMHDR pnmh, BOOL&) {
    NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;
    LVITEM item = plvdi->item;
    switch (item.iSubItem) {
        case 0:
            plvdi->item.pszText = ((Classifier*)item.lParam)->GetName();
            break;
        default:
            break;
    }
    return 0;
}

LRESULT CFilterSelect::OnNameChange(int idCtrl, LPNMHDR pnmh, BOOL&) {
    NMLVDISPINFO* plvdi = (NMLVDISPINFO*)pnmh;
    LVITEM item = plvdi->item;
	if (item.pszText != NULL) {
		switch (item.iSubItem) {
			case 0:
				((Classifier*)item.lParam)->SetName(item.pszText);
				break;
			default:
				break;
		}
		((Classifier*)item.lParam)->Save();
		parent->InvalidateRect(NULL, FALSE);
	}
    return 0;
}

LRESULT CFilterSelect::OnItemActivate(int idCtrl, LPNMHDR pnmh, BOOL&) {
    HWND listView = GetDlgItem(IDC_FILTER_LIST);
    LPNMLISTVIEW nmlv = (LPNMLISTVIEW) pnmh;

    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.iItem = nmlv->iItem;
    lvi.iSubItem = 0;
    if (ListView_GetItem(listView, &lvi)) {

        // filter was double-clicked, so we will set it as the current filter
        Classifier *classifier = (Classifier*) lvi.lParam;

        // need to replace current classifier with this one by calling ReplaceClassifier in parent
        bool isAlreadyLoaded = parent->SendMessage(WM_LOAD_FILTER, ((WPARAM)classifier->classifierType), ((LPARAM)classifier));
		if (isAlreadyLoaded) {
			ListView_EditLabel(listView, lvi.iItem);
		}
    }
    return 0;
}

LRESULT CFilterSelect::OnItemKeyDown(int idCtrl, LPNMHDR pnmh, BOOL&) {
    HWND listView = GetDlgItem(IDC_FILTER_LIST);
    LPNMLVKEYDOWN nmlv = (LPNMLVKEYDOWN) pnmh;

    if (nmlv->wVKey == VK_DELETE) {
        int numSelected = ListView_GetSelectedCount(listView);
        int iSelection = -1;
        for (int iIndex=0; iIndex<numSelected; iIndex++) {

            // find index of next selected item
            iSelection = ListView_GetNextItem(listView, iSelection, LVNI_SELECTED);

            // delete the selected item
            LV_ITEM lvi;
            ZeroMemory(&lvi, sizeof(lvi));
            lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
            lvi.iItem = iSelection;
            lvi.iSubItem = 0;
            if (ListView_GetItem(listView, &lvi)) {
                Classifier *toDelete = (Classifier*) lvi.lParam;
                ListView_DeleteItem(listView, iSelection);
                toDelete->DeleteFromDisk();
                iSelection = -1;
            }
        }
    }
    return 0;
}

void CFilterSelect::SelectFilter(int filterIndex) {
	if ((filterIndex<0) || (filterIndex>=NUM_FILTERS)) return;
    HWND hwndCombo = GetDlgItem(IDC_FILTER_COMBO);
	ComboBox_SetCurSel(hwndCombo, filterIndex);
}

void CFilterSelect::SetThreshold(float threshold) {
	int newPos = threshold * 100;
	SendDlgItemMessage(IDC_FILTER_THRESHOLD, TBM_SETPOS, true, newPos);
}

void CFilterSelect::AddSavedFilter(Classifier* classifier) {
    HWND listView = GetDlgItem(IDC_FILTER_LIST);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.lParam = (LPARAM) classifier;
    lvi.pszText = LPSTR_TEXTCALLBACK;
    ListView_InsertItem(listView, &lvi);
}
