#pragma once
#include <windows.h>
#include <string>
#include "Theme.h"

// ── Reusable GDI drawing helpers ──────────────────────────────────────────────

// Fill rect with solid brush (no DC state leak)
void FillRectColor(HDC hdc, const RECT& rc, COLORREF color);

// Draw a rounded rectangle filled with fillColor and outlined with borderColor
void DrawRoundRect(HDC hdc, RECT rc, int radius,
                   COLORREF fillColor, COLORREF borderColor, int borderPx = 1);

// Draw a single line of text using the given font (must be selected before call)
// DT_VCENTER|DT_SINGLELINE require the rect to have correct height
void DrawText_(HDC hdc, const std::wstring& text, RECT rc,
               COLORREF color, int ptSize = 10, bool bold = false,
               UINT fmt = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

// Draw a simple push-button rectangle (used for custom buttons in non-dialog windows)
void DrawButton(HDC hdc, const RECT& rc, const std::wstring& text,
                COLORREF bgColor, COLORREF textColor, int ptSize = 10);

// Subclass an existing HWND button to paint with theme colors.
// Call once after CreateWindow for a BS_PUSHBUTTON.
void ApplyButtonTheme(HWND hBtn);

// Create a standard Win32 EDIT control styled to match the current theme
HWND CreateThemedEdit(HWND parent, int id, int x, int y, int w, int h,
                      bool multiline = false, DWORD extraStyle = 0);

// Double-buffer helper — call at start of WM_PAINT; returns back-buffer HDC.
// Call EndDoubleBuffer() when done, passing the same struct.
struct PaintBuffer {
    PAINTSTRUCT  ps;
    HDC          hdc;     // front DC
    HDC          memDC;   // back DC
    HBITMAP      bmp;
    HBITMAP      oldBmp;
    RECT         rc;
};
bool BeginDoubleBuffer(HWND hwnd, PaintBuffer& pb);
void EndDoubleBuffer(PaintBuffer& pb);
