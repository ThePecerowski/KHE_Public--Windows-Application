#include "NoteEditorDialog.h"
#include "Utils.h"
#include "Database.h"
#include "Theme.h"
#include "resource.h"
#include <richedit.h>
#include <commdlg.h>
#include <string>

static bool              s_noteOk  = false;
static NoteEditorDialog* s_noteDlg = nullptr;

bool NoteEditorDialog::show(HWND parent, Note& note) {
    m_note   = &note;
    m_isEdit = (note.id > 0);
    s_noteOk  = false;
    s_noteDlg = this;

    struct {
        DLGTEMPLATE tmpl;
        WORD menu, cls;
        WCHAR title[40];
    } dlg{};
    dlg.tmpl.style  = DS_SETFONT | DS_MODALFRAME | DS_CENTER |
                      WS_POPUP   | WS_CAPTION    | WS_SYSMENU | WS_THICKFRAME;
    dlg.tmpl.cx     = 680;
    dlg.tmpl.cy     = 520;
    const wchar_t* t = m_isEdit ? L"Notu Düzenle" : L"Yeni Not";
    wcsncpy_s(dlg.title, t, 39);

    DialogBoxIndirectW(
        (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE),
        &dlg.tmpl, parent, DlgProc);

    return s_noteOk;
}

void NoteEditorDialog::onInit(HWND dlg) {
    SendMessageW(dlg, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(dlg, GWLP_HINSTANCE);

    auto lbl = [&](const wchar_t* txt, int x, int y, int w, int h) {
        HWND hw = CreateWindowW(L"STATIC", txt, WS_CHILD | WS_VISIBLE, x, y, w, h,
            dlg, nullptr, hi, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);
    };
    auto btn = [&](const wchar_t* txt, int id, int x, int y, int w, int h) {
        HWND hw = CreateWindowW(L"BUTTON", txt,
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            x, y, w, h, dlg, (HMENU)(INT_PTR)id, hi, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);
    };

    // Title field
    lbl(L"Başlık:", 12, 12, 60, 20);
    HWND hTitle = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        80, 10, 570, 24,
        dlg, (HMENU)IDC_NOTE_TITLE_EDIT, hi, nullptr);
    SendMessageW(hTitle, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);
    if (!m_note->title.empty())
        SetWindowTextW(hTitle, m_note->title.c_str());

    // ── Formatting toolbar ────────────────────────────────────────────────────
    btn(L"B",  IDC_BTN_FONT_BOLD,      12, 44, 26, 24);
    btn(L"I",  IDC_BTN_FONT_ITALIC,    44, 44, 26, 24);
    btn(L"U",  IDC_BTN_FONT_UNDERLINE, 76, 44, 26, 24);

    lbl(L"|", 108, 44, 10, 24);

    btn(L"🖼 Resim",   IDC_BTN_ADD_IMAGE, 118, 44, 90, 24);
    btn(L"🎬 Video",   IDC_BTN_ADD_VIDEO, 214, 44, 90, 24);
    btn(L"🎵 Ses",     IDC_BTN_ADD_AUDIO, 310, 44, 90, 24);

    // ── Rich-text area ────────────────────────────────────────────────────────
    HWND hRich = CreateWindowExW(WS_EX_CLIENTEDGE,
        L"RICHEDIT50W", nullptr,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL,
        12, 76, 638, 310,
        dlg, (HMENU)IDC_NOTE_RICHTEXT, hi, nullptr);
    SendMessageW(hRich, WM_SETFONT, (WPARAM)Utils::getFont(11), TRUE);
    SendMessageW(hRich, EM_SETEVENTMASK, 0, ENM_CHANGE);

    // Load existing RTF if editing
    if (!m_note->rtfContent.empty()) {
        struct StreamIn { const char* data; size_t size; size_t pos; };
        StreamIn si{ m_note->rtfContent.data(), m_note->rtfContent.size(), 0 };
        EDITSTREAM es{};
        es.dwCookie    = (DWORD_PTR)&si;
        es.pfnCallback = [](DWORD_PTR cookie, LPBYTE buf, LONG cb, LONG* pcb) -> DWORD {
            auto* si = reinterpret_cast<StreamIn*>(cookie);
            size_t avail = si->size - si->pos;
            size_t toRead = (size_t)cb < avail ? (size_t)cb : avail;
            memcpy(buf, si->data + si->pos, toRead);
            si->pos += toRead;
            *pcb = (LONG)toRead;
            return 0;
        };
        SendMessageW(hRich, EM_STREAMIN, SF_RTF, (LPARAM)&es);
    }

    // ── Media list ────────────────────────────────────────────────────────────
    lbl(L"Ekler:", 12, 395, 60, 16);
    HWND hMedia = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
        12, 412, 500, 64,
        dlg, (HMENU)IDC_MEDIA_LIST, hi, nullptr);
    SendMessageW(hMedia, WM_SETFONT, (WPARAM)Utils::getFont(8), TRUE);

    btn(L"Kaldır", IDC_BTN_DELETE_PATH, 520, 412, 70, 26);

    // ── Save / Cancel ─────────────────────────────────────────────────────────
    btn(L"Kaydet", IDOK,     550, 484, 80, 28);
    btn(L"İptal",  IDCANCEL, 638, 484, 62, 28);

    refreshMediaList(dlg);
}

void NoteEditorDialog::applyFormat(HWND dlg, DWORD effect) {
    HWND hRich = GetDlgItem(dlg, IDC_NOTE_RICHTEXT);
    CHARFORMAT2W cf{};
    cf.cbSize    = sizeof(cf);
    cf.dwMask    = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;

    // Get current char format of selection
    SendMessageW(hRich, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Toggle the effect
    if (cf.dwEffects & effect)
        cf.dwEffects &= ~effect;
    else
        cf.dwEffects |=  effect;

    SendMessageW(hRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    SetFocus(hRich);
}

void NoteEditorDialog::onAddMedia(HWND dlg, const wchar_t* type) {
    const wchar_t* filter = L"Tüm Dosyalar\0*.*\0";
    if (std::wstring(type) == L"image")
        filter = L"Resim Dosyaları\0*.jpg;*.jpeg;*.png;*.gif;*.bmp;*.webp\0Tüm\0*.*\0";
    else if (std::wstring(type) == L"video")
        filter = L"Video Dosyaları\0*.mp4;*.avi;*.mkv;*.mov;*.wmv\0Tüm\0*.*\0";
    else if (std::wstring(type) == L"audio")
        filter = L"Ses Dosyaları\0*.mp3;*.wav;*.flac;*.aac;*.ogg\0Tüm\0*.*\0";

    std::wstring path = Utils::browseForFile(dlg, L"Medya Seç", filter);
    if (path.empty()) return;

    NoteMedia med;
    med.noteId    = m_note->id;   // may be 0 for new notes — saved later
    med.mediaPath = path;
    med.mediaType = type;
    m_note->media.push_back(med);
    refreshMediaList(dlg);
}

void NoteEditorDialog::onRemoveMedia(HWND dlg) {
    HWND hList = GetDlgItem(dlg, IDC_MEDIA_LIST);
    LRESULT sel = SendMessageW(hList, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (LRESULT)m_note->media.size()) return;
    m_note->media.erase(m_note->media.begin() + sel);
    refreshMediaList(dlg);
}

void NoteEditorDialog::refreshMediaList(HWND dlg) {
    HWND hList = GetDlgItem(dlg, IDC_MEDIA_LIST);
    SendMessageW(hList, LB_RESETCONTENT, 0, 0);
    for (auto& m : m_note->media) {
        std::wstring lbl = L"[" + m.mediaType + L"]  " + m.mediaPath;
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)lbl.c_str());
    }
}

bool NoteEditorDialog::onSave(HWND dlg) {
    wchar_t buf[512] = {};
    GetDlgItemTextW(dlg, IDC_NOTE_TITLE_EDIT, buf, 512);
    m_note->title = buf;
    if (m_note->title.empty()) {
        MessageBoxW(dlg, L"Lütfen bir başlık girin.", L"Uyarı", MB_ICONWARNING);
        return false;
    }

    // Stream RTF out
    HWND hRich = GetDlgItem(dlg, IDC_NOTE_RICHTEXT);
    std::string rtf;
    struct StreamOut { std::string* buf; };
    StreamOut so{ &rtf };
    EDITSTREAM es{};
    es.dwCookie    = (DWORD_PTR)&so;
    es.pfnCallback = [](DWORD_PTR cookie, LPBYTE buf, LONG cb, LONG* pcb) -> DWORD {
        auto* so = reinterpret_cast<StreamOut*>(cookie);
        so->buf->append(reinterpret_cast<char*>(buf), cb);
        *pcb = cb;
        return 0;
    };
    SendMessageW(hRich, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
    m_note->rtfContent = std::move(rtf);

    return true;
}

INT_PTR CALLBACK NoteEditorDialog::DlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INITDIALOG:
        s_noteDlg->onInit(dlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDOK:
            if (s_noteDlg->onSave(dlg)) {
                s_noteOk = true;
                EndDialog(dlg, IDOK);
            }
            return TRUE;
        case IDCANCEL:
            EndDialog(dlg, IDCANCEL);
            return TRUE;
        case IDC_BTN_FONT_BOLD:
            s_noteDlg->applyFormat(dlg, CFE_BOLD);      return TRUE;
        case IDC_BTN_FONT_ITALIC:
            s_noteDlg->applyFormat(dlg, CFE_ITALIC);    return TRUE;
        case IDC_BTN_FONT_UNDERLINE:
            s_noteDlg->applyFormat(dlg, CFE_UNDERLINE); return TRUE;
        case IDC_BTN_ADD_IMAGE:
            s_noteDlg->onAddMedia(dlg, L"image"); return TRUE;
        case IDC_BTN_ADD_VIDEO:
            s_noteDlg->onAddMedia(dlg, L"video"); return TRUE;
        case IDC_BTN_ADD_AUDIO:
            s_noteDlg->onAddMedia(dlg, L"audio"); return TRUE;
        case IDC_BTN_DELETE_PATH:  // reusing existing ID as "remove media"
            s_noteDlg->onRemoveMedia(dlg); return TRUE;
        }
        break;

    case WM_GETMINMAXINFO: {
        auto* mmi = reinterpret_cast<MINMAXINFO*>(lp);
        mmi->ptMinTrackSize = {540, 420};
        return 0;
    }
    }
    return FALSE;
}
