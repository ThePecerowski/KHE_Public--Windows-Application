#include "Theme.h"

ThemeMode Theme::mode = ThemeMode::Light;

// ── Light theme ───────────────────────────────────────────────────────────────
const ThemeColors Theme::s_light = {
    /* background    */ RGB(0xFF, 0xFF, 0xFF),
    /* surface       */ RGB(0xF5, 0xF7, 0xFA),
    /* card          */ RGB(0xFF, 0xFF, 0xFF),
    /* border        */ RGB(0xE4, 0xE7, 0xEC),
    /* primaryText   */ RGB(0x11, 0x11, 0x11),
    /* secondaryText */ RGB(0x66, 0x70, 0x85),
    /* primaryBlue   */ RGB(0x3B, 0x82, 0xF6),
    /* lightBlue     */ RGB(0x93, 0xC5, 0xFD),
    /* purple        */ RGB(0x8B, 0x5C, 0xF6),
    /* sidebarBg     */ RGB(0x1D, 0x4E, 0xD8),
    /* sidebarHover  */ RGB(0x25, 0x5D, 0xE8),
    /* sidebarActive */ RGB(0x3B, 0x82, 0xF6),
    /* sidebarText   */ RGB(0xFF, 0xFF, 0xFF),
    /* sidebarSecTxt */ RGB(0x93, 0xC5, 0xFD),
    /* topBarBg      */ RGB(0xFF, 0xFF, 0xFF),
    /* topBarText    */ RGB(0x11, 0x11, 0x11),
    /* btnHover      */ RGB(0x2B, 0x6C, 0xE6),
    /* btnActive     */ RGB(0x1D, 0x4E, 0xD8),
    /* danger        */ RGB(0xEF, 0x44, 0x44),
};

// ── Dark theme ────────────────────────────────────────────────────────────────
const ThemeColors Theme::s_dark = {
    /* background    */ RGB(0x0F, 0x17, 0x2A),
    /* surface       */ RGB(0x1E, 0x29, 0x3B),
    /* card          */ RGB(0x1F, 0x29, 0x37),
    /* border        */ RGB(0x33, 0x41, 0x55),
    /* primaryText   */ RGB(0xF8, 0xFA, 0xFC),
    /* secondaryText */ RGB(0x94, 0xA3, 0xB8),
    /* primaryBlue   */ RGB(0x60, 0xA5, 0xFA),
    /* lightBlue     */ RGB(0x93, 0xC5, 0xFD),
    /* purple        */ RGB(0xA7, 0x8B, 0xFA),
    /* sidebarBg     */ RGB(0x0A, 0x10, 0x1E),
    /* sidebarHover  */ RGB(0x1E, 0x29, 0x3B),
    /* sidebarActive */ RGB(0x60, 0xA5, 0xFA),
    /* sidebarText   */ RGB(0xF8, 0xFA, 0xFC),
    /* sidebarSecTxt */ RGB(0x94, 0xA3, 0xB8),
    /* topBarBg      */ RGB(0x1E, 0x29, 0x3B),
    /* topBarText    */ RGB(0xF8, 0xFA, 0xFC),
    /* btnHover      */ RGB(0x3B, 0x82, 0xF6),
    /* btnActive     */ RGB(0x25, 0x63, 0xEB),
    /* danger        */ RGB(0xF8, 0x71, 0x71),
};

const ThemeColors& Theme::colors() {
    return (mode == ThemeMode::Light) ? s_light : s_dark;
}

void Theme::toggle() {
    mode = (mode == ThemeMode::Light) ? ThemeMode::Dark : ThemeMode::Light;
}

void Theme::setMode(ThemeMode m) {
    mode = m;
}
