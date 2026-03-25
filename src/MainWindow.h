#pragma once
#include <windows.h>
#include <string>

// ── Sidebar ───────────────────────────────────────────────────────────────────
// Self-contained sidebar window. Draws navigation items and the theme toggle.
class Sidebar {
public:
    static const int WIDTH = 220;

    bool create(HWND parent, HINSTANCE hi, int height);
    void resize(int height);
    void setActivePage(int page);
    void repaint();

    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onPaint();
    void onMouseMove(int x, int y);
    void onMouseLeave();
    void onLButtonDown(int x, int y);

    // Hit-test: returns page index (0-2) or -1 for theme toggle
    int hitTest(int y) const;

    HWND m_hwnd       = nullptr;
    int  m_activePage = 0;
    int  m_hoverItem  = -2;   // -2 = none, 0-2 = page nav, -1 = theme toggle
    bool m_tracking   = false;
};

// ── TopBar (breadcrumb) ───────────────────────────────────────────────────────
class TopBar {
public:
    static const int HEIGHT = 48;

    bool create(HWND parent, HINSTANCE hi, int x, int width);
    void resize(int x, int width);
    void setPath(const wchar_t* path);
    void repaint();

    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onPaint();

    HWND         m_hwnd = nullptr;
    std::wstring m_path = L"Ana Sayfa";  // breadcrumb text
};

// ── MainWindow ────────────────────────────────────────────────────────────────
class MainWindow {
public:
    bool create(HINSTANCE hi, int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    void onCreate(HWND hwnd);
    void onSize(int w, int h);
    void onNavigate(int page);
    void onThemeChanged();
    void onDestroy();

    // Show only the page window matching |page|; hide others
    void showPage(int page);

    HWND m_hwnd = nullptr;
};

// Breadcrumb labels per page
static const wchar_t* PAGE_LABELS[] = {
    L"Ana Sayfa",
    L"Dosya Yolları",
    L"Notlar",
    L"Not Görüntüle",
};
