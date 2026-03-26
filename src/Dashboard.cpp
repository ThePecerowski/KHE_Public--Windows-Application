#include "Dashboard.h"
#include <algorithm>
#include "AppContext.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "Database.h"
#include "resource.h"

static const wchar_t* DASH_CLASS = L"KHE_Dashboard";

bool Dashboard::create(HWND parent, HINSTANCE hi, int x, int y, int w, int h) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = DASH_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, DASH_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void Dashboard::onCreate(HWND hwnd, int w, int h) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    // ── Quick action buttons ─────────────────────────────────────────────────
    m_hAddPath = CreateWindowW(L"BUTTON", L"+ Hızlı Yol Ekle",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        16, 80, 200, 36,
        hwnd, (HMENU)IDC_BTN_QUICK_ADD_PATH, hi, nullptr);
    SendMessageW(m_hAddPath, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    m_hAddNote = CreateWindowW(L"BUTTON", L"+ Hızlı Not Ekle",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        232, 80, 200, 36,
        hwnd, (HMENU)IDC_BTN_QUICK_ADD_NOTE, hi, nullptr);
    SendMessageW(m_hAddNote, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    // ── Recent paths list ────────────────────────────────────────────────────
    // Dynamic layout: two equal-height lists, quick-action buttons at the bottom
    int listH       = std::max(80, (int)(h - 200) / 2);
    int notesListY  = 72;
    int pathsListY  = notesListY + listH + 30;
    int quickBtnY   = pathsListY + listH + 28;

    // Move quick-action buttons to computed bottom position
    SetWindowPos(m_hAddPath, nullptr, 16,  quickBtnY, 200, 36, SWP_NOZORDER);
    SetWindowPos(m_hAddNote, nullptr, 232, quickBtnY, 200, 36, SWP_NOZORDER);

    // Notes list (top of content area)
    m_hNotesList = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
        16, notesListY, w - 32, listH,
        hwnd, (HMENU)IDC_RECENT_NOTES_LIST, hi, nullptr);
    SendMessageW(m_hNotesList, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    // ── Recent notes list ────────────────────────────────────────────────────
    // Paths list (below notes)
    m_hPathsList = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | LBS_NOTIFY,
        16, pathsListY, w - 32, listH,
        hwnd, (HMENU)IDC_RECENT_PATHS_LIST, hi, nullptr);
    SendMessageW(m_hPathsList, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);
}

void Dashboard::onSize(int w, int h) {
    if (!m_hPathsList) return;

    int listH     = std::max(80, (int)(h - 200) / 2);
    int notesY    = 72;
    int pathsY    = notesY + listH + 30;
    int quickBtnY = pathsY + listH + 28;

    SetWindowPos(m_hNotesList, nullptr, 16, notesY,    w - 32, listH, SWP_NOZORDER);
    SetWindowPos(m_hPathsList, nullptr, 16, pathsY,    w - 32, listH, SWP_NOZORDER);
    SetWindowPos(m_hAddPath,   nullptr, 16, quickBtnY, 200, 36,       SWP_NOZORDER);
    SetWindowPos(m_hAddNote,   nullptr, 232, quickBtnY, 200, 36,      SWP_NOZORDER);

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Dashboard::refreshData() {
    // Populate recent paths
    SendMessageW(m_hPathsList, LB_RESETCONTENT, 0, 0);
    for (auto& p : Database::get().getRecentPaths(5)) {
        std::wstring txt = p.name + L"  —  " + p.path;
        LRESULT idx = SendMessageW(m_hPathsList, LB_ADDSTRING, 0, (LPARAM)txt.c_str());
        SendMessageW(m_hPathsList, LB_SETITEMDATA, idx, (LPARAM)p.id);
    }

    // Populate recent notes
    SendMessageW(m_hNotesList, LB_RESETCONTENT, 0, 0);
    for (auto& n : Database::get().getRecentNotes(5)) {
        std::wstring txt = n.title;
        if (!n.lastModified.empty())
            txt += L"  (" + Utils::formatDateTime(n.lastModified) + L")";
        LRESULT idx = SendMessageW(m_hNotesList, LB_ADDSTRING, 0, (LPARAM)txt.c_str());
        SendMessageW(m_hNotesList, LB_SETITEMDATA, idx, (LPARAM)n.id);
    }
}

void Dashboard::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT rc; GetClientRect(m_hwnd, &rc);

    HDC    mem  = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    const auto& C = Theme::colors();
    FillRectColor(mem, rc, C.background);

    // ── Title ─────────────────────────────────────────────────────────────────
    RECT titleRc{16, 8, rc.right - 16, 36};
    DrawText_(mem, L"Ana Sayfa", titleRc, C.primaryText, 20, true,
              DT_LEFT | DT_TOP | DT_SINGLELINE);

    // ── Subtitle ──────────────────────────────────────────────────────────────
    RECT subtRc{16, 38, rc.right - 16, 60};
    DrawText_(mem, L"Son kullan\u0131lan \u00f6\u011feler ve h\u0131zl\u0131 i\u015flemler", subtRc,
              C.secondaryText, 8, false, DT_LEFT | DT_TOP | DT_SINGLELINE);

    // Dynamic layout metrics (mirrors onCreate/onSize)
    int listHP      = std::max(80, (int)(rc.bottom - 200) / 2);
    int notesListYP = 72;
    int pathsListYP = notesListYP + listHP + 30;
    int quickBtnYP  = pathsListYP + listHP + 28;

    // Section labels with accent separator lines
    auto sectionHdr = [&](const wchar_t* lbl, int listTop, COLORREF accent) {
        RECT hdr{16, listTop - 20, 400, listTop - 2};
        DrawText_(mem, lbl, hdr, C.secondaryText, 8, true,
                  DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
        RECT sep{16, listTop - 3, rc.right - 16, listTop - 2};
        FillRectColor(mem, sep, Utils::blendColor(C.border, accent, 0.4f));
    };
    sectionHdr(L"\u2710  Son Kullan\u0131lan Notlar",  notesListYP, C.primaryBlue);
    sectionHdr(L"\u2605  Son Kullan\u0131lan Yollar",  pathsListYP, C.primaryBlue);
    sectionHdr(L"\u26A1  H\u0131zl\u0131 \u0130\u015flemler", quickBtnYP,  C.purple);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(m_hwnd, &ps);
}

void Dashboard::onCommand(int id) {
    if (id == IDC_BTN_QUICK_ADD_PATH) {
        AppContext::get().navigateTo(PAGE_PATHS);
        // Signal the paths page to open the add dialog
        if (AppContext::get().hPathsPage)
            PostMessageW(AppContext::get().hPathsPage, WM_COMMAND,
                         MAKEWPARAM(IDC_BTN_ADD_PATH, BN_CLICKED), 0);
        return;
    }
    if (id == IDC_BTN_QUICK_ADD_NOTE) {
        AppContext::get().navigateTo(PAGE_NOTES);
        if (AppContext::get().hNotesPage)
            PostMessageW(AppContext::get().hNotesPage, WM_COMMAND,
                         MAKEWPARAM(IDC_BTN_ADD_NOTE, BN_CLICKED), 0);
        return;
    }

    // Double-click on recent path → open path
    if (id == IDC_RECENT_PATHS_LIST) {
        LRESULT sel = SendMessageW(m_hPathsList, LB_GETCURSEL, 0, 0);
        if (sel == LB_ERR) return;
        int pathId = (int)SendMessageW(m_hPathsList, LB_GETITEMDATA, sel, 0);
        auto paths = Database::get().getAllPaths();
        for (auto& p : paths) {
            if (p.id == pathId) {
                Utils::showInExplorer(p.path);
                Database::get().touchPath(pathId);
                break;
            }
        }
        return;
    }

    // Double-click on recent note → view note
    if (id == IDC_RECENT_NOTES_LIST) {
        LRESULT sel = SendMessageW(m_hNotesList, LB_GETCURSEL, 0, 0);
        if (sel == LB_ERR) return;
        int noteId = (int)SendMessageW(m_hNotesList, LB_GETITEMDATA, sel, 0);
        AppContext::get().viewingNoteId = noteId;
        AppContext::get().navigateTo(PAGE_NOTE_VIEWER);
    }
}

LRESULT CALLBACK Dashboard::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Dashboard* self = (Dashboard*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (Dashboard*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE: {
        RECT rc; GetClientRect(hwnd, &rc);
        self->onCreate(hwnd, rc.right, rc.bottom);
        return 0;
    }
    case WM_SIZE:
        self->onSize(LOWORD(lp), HIWORD(lp));
        return 0;
    case WM_PAINT:
        self->onPaint();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_COMMAND:
        self->onCommand(LOWORD(wp));
        return 0;
    case WM_DATA_UPDATED:
        self->refreshData();
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
        SetBkColor(hdc, C.surface);
        static HBRUSH s_btnBr = nullptr;
        if (s_btnBr) DeleteObject(s_btnBr);
        s_btnBr = CreateSolidBrush(C.surface);
        return (LRESULT)s_btnBr;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
