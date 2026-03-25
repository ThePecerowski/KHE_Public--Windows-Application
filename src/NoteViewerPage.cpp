#include "NoteViewerPage.h"
#include "AppContext.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "Database.h"
#include "NoteEditorDialog.h"
#include "resource.h"
#include <richedit.h>
#include <string>

static const wchar_t* VIEWER_CLASS = L"KHE_NoteViewer";

bool NoteViewerPage::create(HWND parent, HINSTANCE hi, int x, int y, int w, int h) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = VIEWER_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, VIEWER_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void NoteViewerPage::onCreate(HWND hwnd, int w, int h) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    // Back button
    m_hBtnBack = CreateWindowW(L"BUTTON", L"← Geri",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        16, 16, 100, 32,
        hwnd, (HMENU)IDC_BTN_BACK, hi, nullptr);
    SendMessageW(m_hBtnBack, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // Edit button
    m_hBtnEdit = CreateWindowW(L"BUTTON", L"✎ Düzenle",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        128, 16, 110, 32,
        hwnd, (HMENU)IDC_BTN_EDIT_NOTE, hi, nullptr);
    SendMessageW(m_hBtnEdit, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // RichEdit (read-only view)
    int richW = w - 32;
    int richH = h - 180;
    m_hRichEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
        L"RICHEDIT50W", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        16, 68, richW, richH,
        hwnd, (HMENU)IDC_VIEWER_RICHTEXT, hi, nullptr);
    SendMessageW(m_hRichEdit, WM_SETFONT, (WPARAM)Utils::getFont(11), TRUE);
    SendMessageW(m_hRichEdit, EM_SETEVENTMASK, 0, ENM_SELCHANGE);

    // Media list
    int mediaY = 68 + richH + 12;
    m_hMediaList = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
        16, mediaY, richW, h - mediaY - 60,
        hwnd, (HMENU)IDC_VIEWER_MEDIA_LIST, hi, nullptr);
    SendMessageW(m_hMediaList, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // Play media button
    m_hBtnPlay = CreateWindowW(L"BUTTON", L"▶ Medyayı Oynat",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        16, h - 48, 170, 32,
        hwnd, (HMENU)IDC_BTN_PLAY_MEDIA, hi, nullptr);
    SendMessageW(m_hBtnPlay, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);
}

void NoteViewerPage::onSize(int w, int h) {
    if (!m_hRichEdit) return;

    SetWindowPos(m_hBtnBack, nullptr,  16,  16, 100, 32, SWP_NOZORDER);
    SetWindowPos(m_hBtnEdit, nullptr, 128,  16, 110, 32, SWP_NOZORDER);

    int richW = w - 32;
    int richH = h - 180;
    SetWindowPos(m_hRichEdit, nullptr, 16, 68, richW, richH, SWP_NOZORDER);

    int mediaY = 68 + richH + 12;
    SetWindowPos(m_hMediaList, nullptr, 16, mediaY, richW,
                 std::max(40, h - mediaY - 60), SWP_NOZORDER);

    SetWindowPos(m_hBtnPlay, nullptr, 16, h - 48, 170, 32, SWP_NOZORDER);

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void NoteViewerPage::loadNote(int noteId) {
    if (noteId <= 0) return;
    m_note = Database::get().getNoteById(noteId);

    // Load RTF into RichEdit
    if (!m_note.rtfContent.empty()) {
        struct StreamIn { const char* data; size_t size; size_t pos; };
        StreamIn si{ m_note.rtfContent.data(), m_note.rtfContent.size(), 0 };

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
        SendMessageW(m_hRichEdit, EM_STREAMIN, SF_RTF, (LPARAM)&es);
    } else {
        SetWindowTextW(m_hRichEdit, m_note.title.c_str());
    }

    // Populate media list
    SendMessageW(m_hMediaList, LB_RESETCONTENT, 0, 0);
    for (auto& med : m_note.media) {
        // Show: [type] filename
        std::wstring label = L"[" + med.mediaType + L"]  " + med.mediaPath;
        LRESULT idx = SendMessageW(m_hMediaList, LB_ADDSTRING, 0, (LPARAM)label.c_str());
        SendMessageW(m_hMediaList, LB_SETITEMDATA, idx, (LPARAM)med.id);
    }

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void NoteViewerPage::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT rc; GetClientRect(m_hwnd, &rc);

    HDC    mem  = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    const auto& C = Theme::colors();
    FillRectColor(mem, rc, C.background);

    // Note title
    if (!m_note.title.empty()) {
        RECT titleRc{140, 16, rc.right - 16, 52};
        DrawText_(mem, m_note.title, titleRc, C.primaryText, 14, true,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    // "Medya" section label
    RECT rc2; GetClientRect(m_hwnd, &rc2);
    int richH = rc2.bottom - 180;
    int mediaLY = 68 + richH;
    RECT mediaHdrRc{16, mediaLY, 200, mediaLY + 14};
    DrawText_(mem, L"Ekler", mediaHdrRc, C.secondaryText, 8, false,
              DT_LEFT | DT_TOP | DT_SINGLELINE);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem);
    EndPaint(m_hwnd, &ps);
}

void NoteViewerPage::onCommand(int id) {
    if (id == IDC_BTN_BACK) {
        AppContext::get().navigateTo(PAGE_NOTES);
        return;
    }
    if (id == IDC_BTN_EDIT_NOTE) {
        NoteEditorDialog dlg;
        Note note = m_note;
        if (dlg.show(m_hwnd, note)) {
            Database::get().updateNote(note);
            loadNote(note.id);
        }
        return;
    }
    if (id == IDC_BTN_PLAY_MEDIA) {
        LRESULT sel = SendMessageW(m_hMediaList, LB_GETCURSEL, 0, 0);
        if (sel == LB_ERR) {
            MessageBoxW(m_hwnd, L"Lütfen oynatmak istediğiniz bir medya seçin.",
                        L"Uyarı", MB_ICONINFORMATION);
            return;
        }
        // Find media by index
        if (sel < (LRESULT)m_note.media.size())
            Utils::playMedia(m_note.media[(size_t)sel].mediaPath);
    }
}

LRESULT CALLBACK NoteViewerPage::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    NoteViewerPage* self = (NoteViewerPage*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (NoteViewerPage*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE: {
        RECT rc; GetClientRect(hwnd, &rc);
        self->onCreate(hwnd, rc.right, rc.bottom);
        return 0;
    }
    case WM_SIZE:    self->onSize(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_PAINT:   self->onPaint();                       return 0;
    case WM_ERASEBKGND: return 1;
    case WM_COMMAND: self->onCommand(LOWORD(wp));           return 0;
    case WM_DATA_UPDATED:
        self->loadNote(AppContext::get().viewingNoteId);
        return 0;
    case WM_THEME_CHANGED:
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.surface);
        static HBRUSH s_br = nullptr;
        if (s_br) DeleteObject(s_br);
        s_br = CreateSolidBrush(C.surface);
        return (LRESULT)s_br;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.background);
        static HBRUSH s_btnBr = nullptr;
        if (s_btnBr) DeleteObject(s_btnBr);
        s_btnBr = CreateSolidBrush(C.background);
        return (LRESULT)s_btnBr;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
