#include "Controls.h"
#include "Utils.h"
#include <algorithm>

// ── Fill helper ───────────────────────────────────────────────────────────────
void FillRectColor(HDC hdc, const RECT& rc, COLORREF color) {
    HBRUSH br = CreateSolidBrush(color);
    FillRect(hdc, &rc, br);
    DeleteObject(br);
}

// ── Rounded rect ──────────────────────────────────────────────────────────────
void DrawRoundRect(HDC hdc, RECT rc, int radius,
                   COLORREF fillColor, COLORREF borderColor, int borderPx) {
    HBRUSH  fillBr   = CreateSolidBrush(fillColor);
    HPEN    borderPen = CreatePen(PS_SOLID, borderPx, borderColor);

    HBRUSH oldBr  = (HBRUSH)SelectObject(hdc, fillBr);
    HPEN   oldPen = (HPEN)SelectObject(hdc, borderPen);

    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius * 2, radius * 2);

    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBr);
    DeleteObject(borderPen);
}

// ── Text draw ─────────────────────────────────────────────────────────────────
void DrawText_(HDC hdc, const std::wstring& text, RECT rc,
               COLORREF color, int ptSize, bool bold, UINT fmt) {
    HFONT  font    = Utils::getFont(ptSize, bold);
    HFONT  oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, text.c_str(), (int)text.size(), &rc, fmt);
    SelectObject(hdc, oldFont);
}

// ── Button draw ───────────────────────────────────────────────────────────────
void DrawButton(HDC hdc, const RECT& rc, const std::wstring& text,
                COLORREF bgColor, COLORREF textColor, int ptSize) {
    DrawRoundRect(hdc, rc, 6, bgColor, bgColor);
    RECT inner = rc;
    InflateRect(&inner, -4, -4);
    DrawText_(hdc, text, inner, textColor, ptSize, false,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

// ── Themed Edit control ───────────────────────────────────────────────────────
HWND CreateThemedEdit(HWND parent, int id, int x, int y, int w, int h,
                      bool multiline, DWORD extraStyle) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | extraStyle;
    if (multiline)
        style = (style & ~ES_AUTOHSCROLL) | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
    HWND hEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT", L"",
        style,
        x, y, w, h,
        parent, (HMENU)(INT_PTR)id,
        (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE),
        nullptr);
    SendMessageW(hEdit, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);
    return hEdit;
}

// ── Double-buffer ─────────────────────────────────────────────────────────────
bool BeginDoubleBuffer(HWND hwnd, PaintBuffer& pb) {
    pb.hdc = BeginPaint(hwnd, &pb.ps);
    GetClientRect(hwnd, &pb.rc);
    pb.memDC  = CreateCompatibleDC(pb.hdc);
    pb.bmp    = CreateCompatibleBitmap(pb.hdc, pb.rc.right, pb.rc.bottom);
    pb.oldBmp = (HBITMAP)SelectObject(pb.memDC, pb.bmp);
    return true;
}

void EndDoubleBuffer(PaintBuffer& pb) {
    BitBlt(pb.hdc, 0, 0, pb.rc.right, pb.rc.bottom, pb.memDC, 0, 0, SRCCOPY);
    SelectObject(pb.memDC, pb.oldBmp);
    DeleteObject(pb.bmp);
    DeleteDC(pb.memDC);
    // Callers should call EndPaint themselves with their HWND
}
