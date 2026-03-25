#pragma once
#include <windows.h>
#include "Models.h"

// Modal dialog for adding or editing a PathEntry.
// Call show(); if it returns true, |entry| has been filled with user input.
class AddPathDialog {
public:
    // Returns true if user confirmed (OK)
    bool show(HWND parent, PathEntry& entry);

private:
    static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
    void onInit(HWND dlg);
    bool onOK(HWND dlg);
    void onBrowse(HWND dlg);

    PathEntry*  m_entry  = nullptr;
    bool        m_isEdit = false;
};
