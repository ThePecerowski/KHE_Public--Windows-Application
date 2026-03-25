#pragma once
#include <windows.h>

enum class ThemeMode { Light, Dark };

struct ThemeColors {
    COLORREF background;
    COLORREF surface;
    COLORREF card;
    COLORREF border;
    COLORREF primaryText;
    COLORREF secondaryText;
    COLORREF primaryBlue;
    COLORREF lightBlue;
    COLORREF purple;
    COLORREF sidebarBg;
    COLORREF sidebarHover;
    COLORREF sidebarActive;
    COLORREF sidebarText;
    COLORREF sidebarSecText;
    COLORREF topBarBg;
    COLORREF topBarText;
    COLORREF btnHover;
    COLORREF btnActive;
    COLORREF danger;
};

// ── Theme singleton ───────────────────────────────────────────────────────────
class Theme {
public:
    static ThemeMode         mode;
    static const ThemeColors& colors();
    static void              toggle();
    static void              setMode(ThemeMode m);

private:
    static const ThemeColors s_light;
    static const ThemeColors s_dark;
};
