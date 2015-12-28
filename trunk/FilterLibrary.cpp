#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"
#include "Classifier.h"
#include "OutputSink.h"
#include "FilterLibrary.h"

CFilterLibrary::CFilterLibrary(CWindow *caller) {
    parent = caller;
}

CFilterLibrary::~CFilterLibrary(void) {
}

LRESULT CFilterLibrary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 0;
}

LRESULT CFilterLibrary::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    parent->SendMessage(uMsg, wParam, lParam);
    return 0;
}

LRESULT CFilterLibrary::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    GetDlgItem(IDC_RUNLIVE).EnableWindow(wParam);
    GetDlgItem(IDC_RUNRECORDED).EnableWindow(wParam);
    GetDlgItem(IDC_RESET).EnableWindow(wParam);
    return 1;
}

LRESULT CFilterLibrary::OnTextCallback(int idCtrl, LPNMHDR pnmh, BOOL&) {
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

LRESULT CFilterLibrary::OnItemActivate(int idCtrl, LPNMHDR pnmh, BOOL&) {
    
    LPNMLISTVIEW nmlv = (LPNMLISTVIEW) pnmh;
    HWND listView = GetDlgItem(idCtrl);
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.iItem = nmlv->iItem;
    lvi.iSubItem = 0;

	if (idCtrl == IDC_MY_FILTER_LIST) {

        // we activated an item from the list of custom filters
        if (ListView_GetItem(listView, &lvi)) {

            // filter was double-clicked, so we will add it to the list of filters
            Classifier *classifier = (Classifier*) lvi.lParam;
            parent->SendMessage(WM_ADD_CUSTOM_FILTER, ((WPARAM)classifier->classifierType), ((LPARAM)classifier));
        }
    } else if (idCtrl == IDC_STD_FILTER_LIST) {
    
        // we activated an item from the list of standard filters
        if (ListView_GetItem(listView, &lvi)) {

            // filter was double-clicked, so we will add it to the list of filters
            Classifier *classifier = (Classifier*) lvi.lParam;
            parent->SendMessage(WM_ADD_STD_FILTER, ((WPARAM)classifier->classifierType), ((LPARAM)classifier));
        }
    } else if (idCtrl == IDC_OUTPUT_LIST) {

        // we activated an item from the list of outputs
        if (ListView_GetItem(listView, &lvi)) {

            // output was double-clicked, so we will add it to the list of outputs
            OutputSink *output = (OutputSink*) lvi.lParam;
            parent->SendMessage(WM_ADD_OUTPUT_SINK, NULL, ((LPARAM)output));
        }
	} else if (idCtrl == IDC_ACTIVE_FILTER_LIST) {

		// we activated an item from the list of currently active filters
        if (ListView_GetItem(listView, &lvi)) {

            // filter was double-clicked, so we will bring up its configuration dialog box
            Classifier *classifier = (Classifier*) lvi.lParam;
			classifier->Configure();
        }
	}
    return 0;
}

LRESULT CFilterLibrary::OnItemKeyDown(int idCtrl, LPNMHDR pnmh, BOOL&) {

    LPNMLVKEYDOWN nmlv = (LPNMLVKEYDOWN) pnmh;
    HWND listView = GetDlgItem(idCtrl);

    if (idCtrl == IDC_ACTIVE_FILTER_LIST) {

        // keypress was on an item in the list of active filters
        if (nmlv->wVKey == VK_DELETE) {

            // TODO: define a custom message to notify the parent of removal of
            // filter from active list (so it can be removed from VideoRunner)
        }
    }
    return 0;
}

void CFilterLibrary::AddFilter(HWND listView, Classifier* classifier) {

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

void CFilterLibrary::RemoveFilter(HWND listView, Classifier* classifier) {
    int numItems = ListView_GetItemCount(listView);
    int iSelection = -1;
    for (int iIndex=0; iIndex<numItems; iIndex++) {

        // find index of next item
        iSelection = ListView_GetNextItem(listView, iSelection, LVNI_ALL);

        // delete the selected item if it matches the classifier passed in
        LV_ITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
        lvi.iItem = iSelection;
        lvi.iSubItem = 0;
        if (ListView_GetItem(listView, &lvi)) {
            Classifier *current = (Classifier*) lvi.lParam;
            if (current == classifier) {
                ListView_DeleteItem(listView, iSelection);
                break;
            }
        }
    }
}

void CFilterLibrary::ClearFilters(HWND listView) {
    ListView_DeleteAllItems(listView);
}

void CFilterLibrary::AddOutput(HWND listView, OutputSink* output) {
    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.lParam = (LPARAM) output;
    lvi.pszText = output->GetName();
    ListView_InsertItem(listView, &lvi);
}

void CFilterLibrary::RemoveOutput(HWND listView, OutputSink* output) {
    int numItems = ListView_GetItemCount(listView);
    int iSelection = -1;
    for (int iIndex=0; iIndex<numItems; iIndex++) {

        // find index of next item
        iSelection = ListView_GetNextItem(listView, iSelection, LVNI_ALL);

        // delete the selected item if it matches the output passed in
        LV_ITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
        lvi.iItem = iSelection;
        lvi.iSubItem = 0;
        if (ListView_GetItem(listView, &lvi)) {
            OutputSink *current = (OutputSink*) lvi.lParam;
            if (current == output) {
                ListView_DeleteItem(listView, iSelection);
                break;
            }
        }
    }
}

void CFilterLibrary::ClearOutputs(HWND listView) {
    ListView_DeleteAllItems(listView);
}
