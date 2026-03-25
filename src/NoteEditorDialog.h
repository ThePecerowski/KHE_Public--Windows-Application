#pragma once
#include <windows.h>
#include "Models.h"

// Modal rich-text note editor.
// Supports: bold / italic / underline formatting, image / video / audio attachment.
// Returns true if the user saved the note.
class NoteEditorDialog {
public:
    bool show(HWND parent, Note& note);

private:
    static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

    void onInit(HWND dlg);
    bool onSave(HWND dlg);
    void onAddMedia(HWND dlg, const wchar_t* type);
    void onRemoveMedia(HWND dlg);
    void applyFormat(HWND dlg, DWORD effect);
    void refreshMediaList(HWND dlg);

    Note*  m_note   = nullptr;
    bool   m_isEdit = false;
};
