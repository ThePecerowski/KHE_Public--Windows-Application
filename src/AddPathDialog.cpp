#include "AddPathDialog.h"
#include "Utils.h"
#include "Theme.h"
#include "Controls.h"
#include "resource.h"
#include <commdlg.h>
#include <string>

// ── Dialog template built at run-time (no .rc dialog) ─────────────────────────
// We build all controls programmatically in WM_INITDIALOG.

// We create the dialog using a minimal in-memory DLGTEMPLATE
static bool s_ok = false;
static AddPathDialog* s_dlgPtr = nullptr;

bool AddPathDialog::show(HWND parent, PathEntry& entry) {
    m_entry  = &entry;
    m_isEdit = (entry.id > 0);
    s_ok     = false;
    s_dlgPtr = this;

    // Build an empty DLGTEMPLATE (we create all controls ourselves)
    struct {
        DLGTEMPLATE tmpl;
        WORD        menu;
        WORD        cls;
        WCHAR       title[32];
    } dlg{};
    dlg.tmpl.style          = DS_SETFONT | DS_MODALFRAME | DS_CENTER |
                              WS_POPUP   | WS_CAPTION    | WS_SYSMENU;
    dlg.tmpl.cx             = 420;
    dlg.tmpl.cy             = 220;
    dlg.tmpl.cdit           = 0;
    const wchar_t* t        = m_isEdit ? L"Yolu Düzenle" : L"Yeni Yol Ekle";
    wcsncpy_s(dlg.title, t, 31);

    DialogBoxIndirectW(
        (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE),
        &dlg.tmpl, parent, DlgProc);

    return s_ok;
}

void AddPathDialog::onInit(HWND dlg) {
    // Set font for dialog
    SendMessageW(dlg, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(dlg, GWLP_HINSTANCE);

    auto makeLabel = [&](const wchar_t* text, int x, int y, int w, int h) {
        HWND hw = CreateWindowW(L"STATIC", text,
            WS_CHILD | WS_VISIBLE, x, y, w, h,
            dlg, nullptr, hi, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);
    };
    auto makeEdit = [&](int id, int x, int y, int w, int h, bool ro = false) -> HWND {
        DWORD ex = ro ? ES_READONLY : 0;
        HWND hw = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ex,
            x, y, w, h,
            dlg, (HMENU)(INT_PTR)id, hi, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);
        return hw;
    };
    auto makeBtn = [&](const wchar_t* text, int id, int x, int y, int w, int h) {
        HWND hw = CreateWindowW(L"BUTTON", text,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            x, y, w, h,
            dlg, (HMENU)(INT_PTR)id, hi, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);
    };

    // -- Ad
    makeLabel(L"Ad:", 12, 12, 60, 20);
    HWND hName = makeEdit(IDC_PATH_NAME_EDIT, 80, 10, 320, 24);
    SetWindowTextW(hName, m_entry->name.c_str());

    // -- Yol
    makeLabel(L"Yol:", 12, 48, 60, 20);
    HWND hPath = makeEdit(IDC_PATH_PATH_EDIT, 80, 46, 240, 24);
    SetWindowTextW(hPath, m_entry->path.c_str());

    // -- Browse
    makeBtn(L"Gözat...", IDC_BTN_BROWSE_FILE, 330, 46, 70, 24);

    // -- Açıklama
    makeLabel(L"Açıklama:", 12, 84, 80, 20);
    HWND hDesc = makeEdit(IDC_PATH_DESC_EDIT, 100, 82, 300, 24);
    SetWindowTextW(hDesc, m_entry->description.c_str());

    // -- Separator note: manual entry hint
    HWND hNote = CreateWindowW(L"STATIC",
        L"Yolu elle girebilir veya Gözat ile seçebilirsiniz.",
        WS_CHILD | WS_VISIBLE, 12, 118, 390, 16,
        dlg, nullptr, hi, nullptr);
    SendMessageW(hNote, WM_SETFONT, (WPARAM)Utils::getFont(8), TRUE);

    // -- OK / Cancel
    makeBtn(L"Kaydet", IDOK,     280, 148, 80, 28);
    makeBtn(L"İptal",  IDCANCEL, 368, 148, 60, 28);
}

bool AddPathDialog::onOK(HWND dlg) {
    wchar_t buf[2048] = {};

    GetDlgItemTextW(dlg, IDC_PATH_NAME_EDIT, buf, 2048);
    m_entry->name = buf;
    if (m_entry->name.empty()) {
        MessageBoxW(dlg, L"Lütfen bir ad girin.", L"Uyarı", MB_ICONWARNING);
        return false;
    }

    GetDlgItemTextW(dlg, IDC_PATH_PATH_EDIT, buf, 2048);
    m_entry->path = buf;
    if (m_entry->path.empty()) {
        MessageBoxW(dlg, L"Lütfen bir yol girin.", L"Uyarı", MB_ICONWARNING);
        return false;
    }

    GetDlgItemTextW(dlg, IDC_PATH_DESC_EDIT, buf, 2048);
    m_entry->description = buf;

    return true;
}

void AddPathDialog::onBrowse(HWND dlg) {
    std::wstring picked = Utils::browseForFile(dlg,
        L"Dosya veya Program Seç", L"Tüm Dosyalar\0*.*\0");
    if (!picked.empty())
        SetDlgItemTextW(dlg, IDC_PATH_PATH_EDIT, picked.c_str());
}

INT_PTR CALLBACK AddPathDialog::DlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INITDIALOG:
        s_dlgPtr->onInit(dlg);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            if (s_dlgPtr->onOK(dlg)) {
                s_ok = true;
                EndDialog(dlg, IDOK);
            }
            return TRUE;
        }
        if (LOWORD(wp) == IDCANCEL) {
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        }
        if (LOWORD(wp) == IDC_BTN_BROWSE_FILE) {
            s_dlgPtr->onBrowse(dlg);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
