#include "QuickLaunch.h"
#include "Database.h"
#include "Theme.h"
#include "Controls.h"
#include "AppContext.h"
#include "Utils.h"
#include "resource.h"
#include <commctrl.h>
#include <algorithm>
#include <cctype>

static const wchar_t* QL_CLASS = L"KHE_QuickLaunch";

// ─────────────────────────────────────────────────────────────────────────────
bool QuickLaunch::create(HINSTANCE hi) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = QL_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.style         = CS_DROPSHADOW;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        QL_CLASS, nullptr,
        WS_POPUP,        // no border — we draw our own
        0, 0, PANEL_W, SEARCH_H + HINT_H,
        nullptr, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void QuickLaunch::show() {
    m_results.clear();
    m_selectedIdx = 0;

    // Center on the primary monitor
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int panelH = SEARCH_H + HINT_H;
    int x = (sw - PANEL_W) / 2;
    int y = (sh / 2) - 100;

    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, PANEL_W, panelH,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

    SetWindowTextW(m_hSearchEdit, L"");
    InvalidateRect(m_hwnd, nullptr, TRUE);

    SetForegroundWindow(m_hwnd);
    SetFocus(m_hSearchEdit);
}

void QuickLaunch::hide() {
    ShowWindow(m_hwnd, SW_HIDE);
}

bool QuickLaunch::isVisible() const {
    return m_hwnd && IsWindowVisible(m_hwnd);
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::onCreate(HWND hwnd) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    // Search edit — positioned after a ~36px icon area
    m_hSearchEdit = CreateWindowExW(0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        44, (SEARCH_H - 28) / 2, PANEL_W - 56, 28,
        hwnd, (HMENU)IDC_QL_SEARCH_EDIT, hi, nullptr);
    SendMessageW(m_hSearchEdit, WM_SETFONT,
                 (WPARAM)Utils::getFont(13, false), TRUE);
    // Placeholder text
    SendMessageW(m_hSearchEdit, EM_SETCUEBANNER, TRUE,
                 (LPARAM)L"Not veya dosya yolu arayın...");

    // Subclass the edit so we can intercept Up/Down/Enter/Escape
    SetWindowSubclass(m_hSearchEdit, SearchEditProc, 1, (DWORD_PTR)this);
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::resizePanel() {
    int visCount = (int)std::min((int)m_results.size(), MAX_VISIBLE);
    int panelH   = SEARCH_H + (visCount > 0 ? visCount * ITEM_H : HINT_H) + 4;

    RECT wr;
    GetWindowRect(m_hwnd, &wr);
    SetWindowPos(m_hwnd, nullptr,
                 wr.left, wr.top, PANEL_W, panelH,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    HDC     memDC = CreateCompatibleDC(hdc);
    HBITMAP bmp   = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old   = (HBITMAP)SelectObject(memDC, bmp);

    const auto& C = Theme::colors();

    // Outer border
    FillRectColor(memDC, rc, C.border);

    // Inner fill for search area
    RECT inner{1, 1, rc.right - 1, SEARCH_H};
    FillRectColor(memDC, inner, C.card);

    // Search icon "⌕" area
    RECT iconRc{0, 0, 44, SEARCH_H};
    DrawText_(memDC, L"⚲", iconRc, C.secondaryText,
              14, false, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Separator below search bar
    RECT sep{1, SEARCH_H, rc.right - 1, SEARCH_H + 1};
    FillRectColor(memDC, sep, C.border);

    // Results area background
    if (!m_results.empty()) {
        RECT resArea{1, SEARCH_H + 1, rc.right - 1, rc.bottom - 1};
        FillRectColor(memDC, resArea, C.surface);

        int visible = std::min((int)m_results.size(), MAX_VISIBLE);
        for (int i = 0; i < visible; ++i) {
            RECT row{1, SEARCH_H + 1 + i * ITEM_H,
                     rc.right - 1, SEARCH_H + 1 + (i + 1) * ITEM_H};

            // Selection highlight
            if (i == m_selectedIdx) {
                FillRectColor(memDC, row,
                    Utils::blendColor(C.surface, C.primaryBlue, 0.12f));
                RECT accent{1, row.top + 4, 4, row.bottom - 4};
                FillRectColor(memDC, accent, C.primaryBlue);
            }

            const auto& r = m_results[i];
            const wchar_t* icon = (r.type == QuickLaunchResult::Type::Note)
                                  ? L"\u270E" : L"\u2397";
            COLORREF iconColor  = (r.type == QuickLaunchResult::Type::Note)
                                  ? C.purple : C.primaryBlue;

            RECT iconRc2{6, row.top, 38, row.bottom};
            DrawText_(memDC, icon, iconRc2, iconColor, 12, false,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT nameRc{44, row.top + 8, rc.right - 12, row.top + 30};
            DrawText_(memDC, r.name, nameRc, C.primaryText, 10, true,
                      DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

            if (!r.secondary.empty()) {
                RECT secRc{44, row.top + 30, rc.right - 12, row.bottom - 4};
                DrawText_(memDC, r.secondary, secRc, C.secondaryText, 9, false,
                          DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
            }

            // Row divider
            if (i < visible - 1) {
                RECT div{44, row.bottom - 1, rc.right - 12, row.bottom};
                FillRectColor(memDC, div, C.border);
            }
        }
    } else {
        // Hint / empty state
        RECT hArea{1, SEARCH_H + 1, rc.right - 1, rc.bottom - 1};
        FillRectColor(memDC, hArea, C.surface);

        int editLen = GetWindowTextLengthW(m_hSearchEdit);
        const wchar_t* hint = (editLen > 0)
            ? L"Sonuç bulunamadı."
            : L"Ara: not başlığı veya dosya adı...";
        DrawText_(memDC, hint, hArea, C.secondaryText, 10, false,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, old);
    DeleteObject(bmp);
    DeleteDC(memDC);
    EndPaint(m_hwnd, &ps);
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::onSearchChanged() {
    wchar_t buf[512]{};
    GetWindowTextW(m_hSearchEdit, buf, 511);
    performSearch(buf);
}

void QuickLaunch::onKeyDown(UINT vk) {
    switch (vk) {
    case VK_ESCAPE:
        hide();
        if (AppContext::get().mainWindow)
            SetForegroundWindow(AppContext::get().mainWindow);
        break;
    case VK_UP:
        if (m_selectedIdx > 0) {
            --m_selectedIdx;
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
        break;
    case VK_DOWN:
        if (m_selectedIdx < (int)m_results.size() - 1) {
            ++m_selectedIdx;
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
        break;
    case VK_RETURN:
        openSelected();
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::performSearch(const std::wstring& query) {
    m_results.clear();
    m_selectedIdx = 0;

    if (query.empty()) {
        resizePanel();
        InvalidateRect(m_hwnd, nullptr, TRUE);
        return;
    }

    // Lower-case query for case-insensitive match
    std::wstring q = query;
    for (auto& c : q) c = (wchar_t)towlower(c);

    // Search notes
    for (auto& note : Database::get().getAllNotes()) {
        std::wstring low = note.title;
        for (auto& c : low) c = (wchar_t)towlower(c);
        if (low.find(q) != std::wstring::npos) {
            QuickLaunchResult r;
            r.type       = QuickLaunchResult::Type::Note;
            r.id         = note.id;
            r.name       = note.title;
            r.secondary  = Utils::formatDateTime(note.lastModified);
            r.openTarget = L"";  // not used for notes
            m_results.push_back(std::move(r));
            if ((int)m_results.size() >= MAX_VISIBLE) break;
        }
    }

    // Search paths (if still room)
    if ((int)m_results.size() < MAX_VISIBLE) {
        for (auto& path : Database::get().getAllPaths()) {
            std::wstring lname = path.name, lpath = path.path;
            for (auto& c : lname) c = (wchar_t)towlower(c);
            for (auto& c : lpath) c = (wchar_t)towlower(c);
            if (lname.find(q) != std::wstring::npos ||
                lpath.find(q) != std::wstring::npos) {
                QuickLaunchResult r;
                r.type       = QuickLaunchResult::Type::Path;
                r.id         = path.id;
                r.name       = path.name;
                r.secondary  = path.path;
                r.openTarget = path.path;
                m_results.push_back(std::move(r));
                if ((int)m_results.size() >= MAX_VISIBLE) break;
            }
        }
    }

    resizePanel();
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

// ─────────────────────────────────────────────────────────────────────────────
void QuickLaunch::openSelected() {
    if (m_selectedIdx < 0 || m_selectedIdx >= (int)m_results.size()) {
        hide();
        return;
    }
    const auto& r = m_results[m_selectedIdx];
    hide();

    if (AppContext::get().mainWindow)
        SetForegroundWindow(AppContext::get().mainWindow);

    if (r.type == QuickLaunchResult::Type::Note) {
        AppContext::get().viewingNoteId = r.id;
        PostMessageW(AppContext::get().mainWindow,
                     WM_NAVIGATE, (WPARAM)PAGE_NOTE_VIEWER, 0);
    } else {
        Database::get().touchPath(r.id);
        Utils::openFile(r.openTarget);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK QuickLaunch::WndProc(HWND hwnd, UINT msg,
                                       WPARAM wp, LPARAM lp) {
    QuickLaunch* self =
        (QuickLaunch*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (QuickLaunch*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE:
        self->onCreate(hwnd);
        return 0;

    case WM_PAINT:
        self->onPaint();
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_COMMAND:
        if (LOWORD(wp) == IDC_QL_SEARCH_EDIT && HIWORD(wp) == EN_CHANGE)
            self->onSearchChanged();
        return 0;

    case WM_ACTIVATE:
        // Hide when focus moves to another window
        if (LOWORD(wp) == WA_INACTIVE)
            self->hide();
        return 0;

    case WM_THEME_CHANGED:
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;

    // Allow EDIT child to inherit theme colours
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.card);
        static HBRUSH s_br = nullptr;
        if (s_br) DeleteObject(s_br);
        s_br = CreateSolidBrush(C.card);
        return (LRESULT)s_br;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Subclass proc: intercepts navigation/confirmation keys from the EDIT
LRESULT CALLBACK QuickLaunch::SearchEditProc(
    HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
    UINT_PTR /*uId*/, DWORD_PTR dwRefData)
{
    auto* self = reinterpret_cast<QuickLaunch*>(dwRefData);
    if (msg == WM_KEYDOWN) {
        UINT vk = (UINT)wp;
        if (vk == VK_ESCAPE || vk == VK_UP ||
            vk == VK_DOWN   || vk == VK_RETURN) {
            self->onKeyDown(vk);
            return 0;
        }
    }
    // WM_NCDESTROY: remove subclass to avoid dangling reference
    if (msg == WM_NCDESTROY)
        RemoveWindowSubclass(hwnd, SearchEditProc, 1);
    return DefSubclassProc(hwnd, msg, wp, lp);
}
