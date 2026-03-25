#include "MainWindow.h"
#include "AppContext.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "resource.h"
#include "Dashboard.h"
#include "PathsPage.h"
#include "NotesPage.h"
#include "NoteViewerPage.h"
#include <string>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// Sidebar
// ─────────────────────────────────────────────────────────────────────────────
static const wchar_t* SB_CLASS = L"KHE_Sidebar";

struct NavItem { const wchar_t* icon; const wchar_t* label; int page; };
static const NavItem NAV_ITEMS[] = {
    { L"⌂", L"Ana Sayfa",       0 },
    { L"⎗", L"Dosya Yolları",   1 },
    { L"✎", L"Notlar",          2 },
};
static constexpr int NAV_ITEM_H   = 48;
static constexpr int LOGO_H       = 64;
static constexpr int TOGGLE_H     = 52;

bool Sidebar::create(HWND parent, HINSTANCE hi, int height) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = SB_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.style         = CS_DBLCLKS;
    RegisterClassExW(&wc);   // ignore if already registered

    m_hwnd = CreateWindowExW(0, SB_CLASS, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, WIDTH, height,
        parent, (HMENU)IDC_SIDEBAR, hi, this);
    return m_hwnd != nullptr;
}

void Sidebar::resize(int height) {
    SetWindowPos(m_hwnd, nullptr, 0, 0, WIDTH, height, SWP_NOMOVE | SWP_NOZORDER);
}

void Sidebar::setActivePage(int page) {
    m_activePage = page;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Sidebar::repaint() { InvalidateRect(m_hwnd, nullptr, FALSE); }

int Sidebar::hitTest(int y) const {
    if (y >= LOGO_H) {
        int rel = y - LOGO_H;
        int idx = rel / NAV_ITEM_H;
        if (idx >= 0 && idx < 3) return idx;
    }
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int toggleTop = rc.bottom - TOGGLE_H - 8;
    if (y >= toggleTop && y < toggleTop + TOGGLE_H) return -1; // theme toggle
    return -2;
}

void Sidebar::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    // Back buffer
    HDC    memDC  = CreateCompatibleDC(hdc);
    HBITMAP bmp   = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old   = (HBITMAP)SelectObject(memDC, bmp);

    const auto& C = Theme::colors();
    FillRectColor(memDC, rc, C.sidebarBg);

    // ── Logo / title ──────────────────────────────────────────────────────────
    RECT logoRc{0, 0, WIDTH, LOGO_H};
    // Slightly darker band
    COLORREF logoBg = Utils::blendColor(C.sidebarBg, RGB(0,0,0), 0.15f);
    FillRectColor(memDC, logoRc, logoBg);
    DrawText_(memDC, L"KHE", logoRc, C.sidebarText, 18, true,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // ── Nav items ─────────────────────────────────────────────────────────────
    for (int i = 0; i < 3; ++i) {
        RECT itemRc{0, LOGO_H + i * NAV_ITEM_H, WIDTH, LOGO_H + (i+1) * NAV_ITEM_H};

        // Highlight active
        if (i == m_activePage) {
            COLORREF activeBg = Utils::blendColor(C.sidebarBg, C.sidebarActive, 0.25f);
            FillRectColor(memDC, itemRc, activeBg);
            // Left accent bar
            RECT accent{0, itemRc.top + 4, 4, itemRc.bottom - 4};
            FillRectColor(memDC, accent, C.sidebarActive);
        } else if (i == m_hoverItem) {
            FillRectColor(memDC, itemRc, C.sidebarHover);
        }

        // Icon
        RECT iconRc{12, itemRc.top, 40, itemRc.bottom};
        DrawText_(memDC, NAV_ITEMS[i].icon, iconRc,
                  (i == m_activePage) ? C.sidebarText : C.sidebarSecText,
                  12, false, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Label
        RECT labelRc{44, itemRc.top, WIDTH - 8, itemRc.bottom};
        DrawText_(memDC, NAV_ITEMS[i].label, labelRc,
                  (i == m_activePage) ? C.sidebarText : C.sidebarSecText,
                  10, (i == m_activePage), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    // Separator
    int sepY = LOGO_H + 3 * NAV_ITEM_H + 8;
    RECT sepRc{12, sepY, WIDTH - 12, sepY + 1};
    FillRectColor(memDC, sepRc, Utils::blendColor(C.sidebarBg, C.sidebarText, 0.2f));

    // ── Theme toggle button ────────────────────────────────────────────────────
    RECT clientRc;
    GetClientRect(m_hwnd, &clientRc);
    int toggleTop = clientRc.bottom - TOGGLE_H - 8;
    RECT toggleRc{12, toggleTop, WIDTH - 12, toggleTop + TOGGLE_H - 8};

    COLORREF toggleBg = (m_hoverItem == -1)
        ? C.sidebarHover
        : Utils::blendColor(C.sidebarBg, C.sidebarActive, 0.15f);
    DrawRoundRect(memDC, toggleRc, 8, toggleBg, C.sidebarActive, 1);

    bool isDark = (Theme::mode == ThemeMode::Dark);
    const wchar_t* toggleLabel = isDark ? L"☀  Aydınlık Tema" : L"☾  Karanlık Tema";
    DrawText_(memDC, toggleLabel, toggleRc, C.sidebarText,
              9, false, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, old);
    DeleteObject(bmp);
    DeleteDC(memDC);
    EndPaint(m_hwnd, &ps);
}

void Sidebar::onMouseMove(int /*x*/, int y) {
    if (!m_tracking) {
        TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, m_hwnd, 0};
        TrackMouseEvent(&tme);
        m_tracking = true;
    }
    int hit = hitTest(y);
    if (hit != m_hoverItem) {
        m_hoverItem = hit;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Sidebar::onMouseLeave() {
    m_tracking  = false;
    m_hoverItem = -2;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Sidebar::onLButtonDown(int /*x*/, int y) {
    int hit = hitTest(y);
    if (hit >= 0 && hit <= 2) {
        // Navigate
        PostMessageW(GetParent(m_hwnd), WM_NAVIGATE, (WPARAM)hit, 0);
    } else if (hit == -1) {
        // Toggle theme
        Theme::toggle();
        PostMessageW(GetParent(m_hwnd), WM_THEME_CHANGED, 0, 0);
    }
}

LRESULT CALLBACK Sidebar::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Sidebar* self = (Sidebar*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (Sidebar*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_PAINT:      self->onPaint();                        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_MOUSEMOVE:  self->onMouseMove(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_MOUSELEAVE: self->onMouseLeave();                   return 0;
    case WM_LBUTTONDOWN: self->onLButtonDown(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_THEME_CHANGED: InvalidateRect(hwnd, nullptr, FALSE); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ─────────────────────────────────────────────────────────────────────────────
// TopBar
// ─────────────────────────────────────────────────────────────────────────────
static const wchar_t* TB_CLASS = L"KHE_TopBar";

bool TopBar::create(HWND parent, HINSTANCE hi, int x, int width) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = TB_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, TB_CLASS, nullptr,
        WS_CHILD | WS_VISIBLE,
        x, 0, width, HEIGHT,
        parent, (HMENU)IDC_BREADCRUMB, hi, this);
    return m_hwnd != nullptr;
}

void TopBar::resize(int x, int width) {
    SetWindowPos(m_hwnd, nullptr, x, 0, width, HEIGHT, SWP_NOZORDER);
}

void TopBar::setPath(const wchar_t* path) {
    m_path = path ? path : L"";
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void TopBar::repaint() { InvalidateRect(m_hwnd, nullptr, FALSE); }

void TopBar::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    HDC    memDC  = CreateCompatibleDC(hdc);
    HBITMAP bmp   = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old   = (HBITMAP)SelectObject(memDC, bmp);

    const auto& C = Theme::colors();
    FillRectColor(memDC, rc, C.topBarBg);

    // Bottom border
    RECT border{rc.left, rc.bottom - 1, rc.right, rc.bottom};
    FillRectColor(memDC, border, C.border);

    // Breadcrumb path
    RECT textRc{16, 0, rc.right - 16, rc.bottom};
    DrawText_(memDC, m_path, textRc, C.secondaryText,
              9, false, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, old);
    DeleteObject(bmp);
    DeleteDC(memDC);
    EndPaint(m_hwnd, &ps);
}

LRESULT CALLBACK TopBar::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    TopBar* self = (TopBar*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (TopBar*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_PAINT:       self->onPaint();                      return 0;
    case WM_ERASEBKGND:  return 1;
    case WM_THEME_CHANGED: InvalidateRect(hwnd, nullptr, FALSE); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ─────────────────────────────────────────────────────────────────────────────
// MainWindow
// ─────────────────────────────────────────────────────────────────────────────
static const wchar_t* MW_CLASS = L"KHE_Main";

// Forward-declared page constructors live in their own .cpp files.
// We keep one of each as static objects referenced via AppContext.
static Sidebar    g_sidebar;
static TopBar     g_topbar;
static Dashboard  g_dashboard;
static PathsPage  g_pathsPage;
static NotesPage  g_notesPage;
static NoteViewerPage g_noteViewer;

bool MainWindow::create(HINSTANCE hi, int nCmdShow) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = MW_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;  // we paint ourselves
    wc.style         = CS_DBLCLKS;
    if (!RegisterClassExW(&wc)) return false;

    m_hwnd = CreateWindowExW(0, MW_CLASS,
        L"KHE — Kişisel Hızlı Erişim Sistemi",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
        nullptr, nullptr, hi, this);
    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

void MainWindow::onCreate(HWND hwnd) {
    m_hwnd = hwnd;
    auto& ctx = AppContext::get();
    ctx.mainWindow = hwnd;

    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int cw = rc.right, ch = rc.bottom;

    // Sidebar
    g_sidebar.create(hwnd, hi, ch);
    ctx.hSidebar = g_sidebar.hwnd();

    // Top bar
    g_topbar.create(hwnd, hi, Sidebar::WIDTH, cw - Sidebar::WIDTH);
    ctx.hTopBar = g_topbar.hwnd();

    int px = Sidebar::WIDTH;
    int py = TopBar::HEIGHT;
    int pw = cw - Sidebar::WIDTH;
    int ph = ch - TopBar::HEIGHT;

    // Page windows
    g_dashboard.create(hwnd, hi, px, py, pw, ph);
    ctx.hDashboard = g_dashboard.hwnd();

    g_pathsPage.create(hwnd, hi, px, py, pw, ph);
    ctx.hPathsPage = g_pathsPage.hwnd();

    g_notesPage.create(hwnd, hi, px, py, pw, ph);
    ctx.hNotesPage = g_notesPage.hwnd();

    g_noteViewer.create(hwnd, hi, px, py, pw, ph);
    ctx.hNoteViewer = g_noteViewer.hwnd();

    // Show dashboard initially
    onNavigate(PAGE_DASHBOARD);
}

void MainWindow::onSize(int w, int h) {
    g_sidebar.resize(h);
    g_topbar.resize(Sidebar::WIDTH, w - Sidebar::WIDTH);

    int px = Sidebar::WIDTH;
    int py = TopBar::HEIGHT;
    int pw = w - Sidebar::WIDTH;
    int ph = h - TopBar::HEIGHT;

    HWND pages[] = {
        AppContext::get().hDashboard,
        AppContext::get().hPathsPage,
        AppContext::get().hNotesPage,
        AppContext::get().hNoteViewer,
    };
    for (HWND p : pages)
        if (p) SetWindowPos(p, nullptr, px, py, pw, ph, SWP_NOZORDER | SWP_NOACTIVATE);
}

void MainWindow::showPage(int page) {
    HWND pages[] = {
        AppContext::get().hDashboard,
        AppContext::get().hPathsPage,
        AppContext::get().hNotesPage,
        AppContext::get().hNoteViewer,
    };
    for (int i = 0; i < 4; ++i)
        if (pages[i]) ShowWindow(pages[i], (i == page) ? SW_SHOW : SW_HIDE);
}

void MainWindow::onNavigate(int page) {
    AppContext::get().currentPage = page;
    showPage(page);
    g_sidebar.setActivePage(page < 3 ? page : 2);
    if (page < 4) g_topbar.setPath(PAGE_LABELS[page]);
    // Notify the now-visible page to refresh data
    PostMessageW(m_hwnd, WM_DATA_UPDATED, (WPARAM)page, 0);
}

void MainWindow::onThemeChanged() {
    // Repaint all windows
    HWND all[] = {
        m_hwnd,
        g_sidebar.hwnd(), g_topbar.hwnd(),
        AppContext::get().hDashboard,
        AppContext::get().hPathsPage,
        AppContext::get().hNotesPage,
        AppContext::get().hNoteViewer,
    };
    for (HWND hw : all)
        if (hw) {
            SendMessageW(hw, WM_THEME_CHANGED, 0, 0);
            InvalidateRect(hw, nullptr, TRUE);
        }
    // Also update child controls' background colours via WM_CTLCOLOR — they'll
    // refresh on next paint.
    RedrawWindow(m_hwnd, nullptr, nullptr,
                 RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

void MainWindow::onDestroy() {
    PostQuitMessage(0);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    MainWindow* self = (MainWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (MainWindow*)cs->lpCreateParams;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE:
        self->onCreate(hwnd);
        return 0;

    case WM_SIZE:
        if (wp != SIZE_MINIMIZED)
            self->onSize(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_NAVIGATE:
        self->onNavigate((int)wp);
        return 0;

    case WM_THEME_CHANGED:
        self->onThemeChanged();
        return 0;

    case WM_DATA_UPDATED: {
        // Forward to the active page so it refreshes its data
        int page = AppContext::get().currentPage;
        HWND pages[] = {
            AppContext::get().hDashboard,
            AppContext::get().hPathsPage,
            AppContext::get().hNotesPage,
            AppContext::get().hNoteViewer,
        };
        if (page < 4 && pages[page])
            SendMessageW(pages[page], WM_DATA_UPDATED, wp, lp);
        return 0;
    }

    case WM_ERASEBKGND: {
        // Fill background with theme colour (visible between child windows)
        HDC hdc = (HDC)wp;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRectColor(hdc, rc, Theme::colors().background);
        return 1;
    }

    case WM_COMMAND:
        if (LOWORD(wp) == IDM_FILE_EXIT)   DestroyWindow(hwnd);
        if (LOWORD(wp) == IDM_HELP_ABOUT)
            MessageBoxW(hwnd,
                L"KHE — Kişisel Hızlı Erişim Sistemi\nSürüm 1.0",
                L"Hakkında", MB_ICONINFORMATION);
        return 0;

    case WM_DESTROY:
        self->onDestroy();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ── AppContext helpers ────────────────────────────────────────────────────────
void AppContext::navigateTo(int page) {
    if (mainWindow) PostMessageW(mainWindow, WM_NAVIGATE, (WPARAM)page, 0);
}
void AppContext::broadcastThemeChange() {
    if (mainWindow) PostMessageW(mainWindow, WM_THEME_CHANGED, 0, 0);
}
void AppContext::broadcastDataUpdate() {
    if (mainWindow) PostMessageW(mainWindow, WM_DATA_UPDATED, 0, 0);
}
