#include "SettingsPage.h"
#include "AppContext.h"
#include "Database.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "resource.h"
#include <sstream>
#include <algorithm>

static const wchar_t* SETTINGS_CLASS = L"KHE_Settings";

// ─────────────────────────────────────────────────────────────────────────────
// Row hit-test helper
int SettingsPage::hitTestRow(int y) const {
    int listTop = HDR_H;
    int total   = (int)m_shortcuts.size();
    if (y < listTop) return -1;
    int idx = (y - listTop) / ROW_H;
    return (idx >= 0 && idx < total) ? idx : -1;
}

// ─────────────────────────────────────────────────────────────────────────────
std::wstring SettingsPage::describeKey(UINT vk, UINT mods) const {
    if (vk == 0) return L"(Atanmamış)";

    std::wstring s;
    if (mods & MOD_CONTROL) s += L"Ctrl + ";
    if (mods & MOD_ALT)     s += L"Alt + ";
    if (mods & MOD_SHIFT)   s += L"Shift + ";

    // Common virtual-key names
    if (vk >= 'A' && vk <= 'Z') {
        s += (wchar_t)vk;
    } else if (vk >= VK_F1 && vk <= VK_F24) {
        s += L"F";
        s += std::to_wstring(vk - VK_F1 + 1);
    } else {
        switch (vk) {
        case VK_SPACE:  s += L"Space";    break;
        case VK_TAB:    s += L"Tab";      break;
        case VK_RETURN: s += L"Enter";    break;
        case VK_ESCAPE: s += L"Esc";      break;
        case VK_LEFT:   s += L"Sol";      break;
        case VK_RIGHT:  s += L"Sag";      break;
        case VK_UP:     s += L"Yukari";   break;
        case VK_DOWN:   s += L"Asagi";    break;
        case VK_DELETE: s += L"Del";      break;
        case VK_INSERT: s += L"Ins";      break;
        case VK_HOME:   s += L"Home";     break;
        case VK_END:    s += L"End";      break;
        default: {
            wchar_t name[64]{};
            UINT scanCode = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
            GetKeyNameTextW((LONG)(scanCode << 16), name, 63);
            if (name[0]) s += name;
            else { s += L"VK("; s += std::to_wstring(vk); s += L")"; }
        }
        }
    }
    return s;
}

std::wstring SettingsPage::describePhase(const ShortcutConfig& cfg) const {
    if (cfg.keyPhase == L"hold") return L"Basılı Tut";
    if (cfg.keyPhase == L"tap" ) {
        return L"" + std::to_wstring(cfg.tapCount) + L" Kez Bas";
    }
    return L"Bas (Anlık)";
}

// ─────────────────────────────────────────────────────────────────────────────
bool SettingsPage::create(HWND parent, HINSTANCE hi, int x, int y, int w, int h) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = SETTINGS_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, SETTINGS_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void SettingsPage::onCreate(HWND hwnd, int w, int h) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    m_shortcuts = Database::get().getAllShortcuts();

    // ── Compute edit panel Y position ─────────────────────────────────────────
    int editPanelY = HDR_H + (int)m_shortcuts.size() * ROW_H + 16;
    int ep         = EDIT_PAD;

    // ── Record button ──────────────────────────────────────────────────────────
    m_hBtnRecord = CreateWindowW(L"BUTTON", L"⏺ Kayıt Başlat",
        WS_CHILD | BS_PUSHBUTTON,
        ep, editPanelY, 160, 34,
        hwnd, (HMENU)IDC_BTN_RECORD_KEY, hi, nullptr);
    SendMessageW(m_hBtnRecord, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // ── Save / Cancel buttons ──────────────────────────────────────────────────
    m_hBtnSave = CreateWindowW(L"BUTTON", L"Kaydet",
        WS_CHILD | BS_PUSHBUTTON,
        ep + 170, editPanelY, 90, 34,
        hwnd, (HMENU)IDC_BTN_SAVE_SHORTCUT, hi, nullptr);
    SendMessageW(m_hBtnSave, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    m_hBtnCancel = CreateWindowW(L"BUTTON", L"İptal",
        WS_CHILD | BS_PUSHBUTTON,
        ep + 270, editPanelY, 90, 34,
        hwnd, (HMENU)IDC_BTN_CANCEL_RECORD, hi, nullptr);
    SendMessageW(m_hBtnCancel, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // ── Phase radio buttons ────────────────────────────────────────────────────
    int phaseY = editPanelY + 50;
    m_hPhaseDown = CreateWindowW(L"BUTTON", L"Bas (Anlık)",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
        ep, phaseY, 140, 26,
        hwnd, (HMENU)IDC_PHASE_DOWN_RADIO, hi, nullptr);
    SendMessageW(m_hPhaseDown, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    m_hPhaseHold = CreateWindowW(L"BUTTON", L"Basılı Tut",
        WS_CHILD | BS_AUTORADIOBUTTON,
        ep + 148, phaseY, 130, 26,
        hwnd, (HMENU)IDC_PHASE_HOLD_RADIO, hi, nullptr);
    SendMessageW(m_hPhaseHold, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    m_hPhaseTap = CreateWindowW(L"BUTTON", L"X Kez Bas",
        WS_CHILD | BS_AUTORADIOBUTTON,
        ep + 286, phaseY, 120, 26,
        hwnd, (HMENU)IDC_PHASE_TAP_RADIO, hi, nullptr);
    SendMessageW(m_hPhaseTap, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // ── Tap count edit ─────────────────────────────────────────────────────────
    m_hTapCount = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"2",
        WS_CHILD | ES_NUMBER | ES_CENTER,
        ep + 414, phaseY, 50, 26,
        hwnd, (HMENU)IDC_TAP_COUNT_EDIT, hi, nullptr);
    SendMessageW(m_hTapCount, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // ── Reset button ───────────────────────────────────────────────────────────
    m_hBtnReset = CreateWindowW(L"BUTTON", L"Varsayılana Sıfırla",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        ep, phaseY + 44, 180, 32,
        hwnd, (HMENU)IDC_BTN_RESET_SHORTCUTS, hi, nullptr);
    SendMessageW(m_hBtnReset, WM_SETFONT, (WPARAM)Utils::getFont(9), TRUE);

    // All editing controls are hidden until a row is selected
    updateEditPanel();
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::onSize(int w, int h) {
    int editPanelY = HDR_H + (int)m_shortcuts.size() * ROW_H + 16;
    int ep = EDIT_PAD;
    int phaseY = editPanelY + 50;

    if (m_hBtnRecord)  SetWindowPos(m_hBtnRecord,  nullptr, ep,             editPanelY, 160, 34, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hBtnSave)    SetWindowPos(m_hBtnSave,    nullptr, ep + 170,       editPanelY, 90,  34, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hBtnCancel)  SetWindowPos(m_hBtnCancel,  nullptr, ep + 270,       editPanelY, 90,  34, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hPhaseDown)  SetWindowPos(m_hPhaseDown,  nullptr, ep,             phaseY,     140, 26, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hPhaseHold)  SetWindowPos(m_hPhaseHold,  nullptr, ep + 148,       phaseY,     130, 26, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hPhaseTap)   SetWindowPos(m_hPhaseTap,   nullptr, ep + 286,       phaseY,     120, 26, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hTapCount)   SetWindowPos(m_hTapCount,   nullptr, ep + 414,       phaseY,     50,  26, SWP_NOZORDER | SWP_NOACTIVATE);
    if (m_hBtnReset)   SetWindowPos(m_hBtnReset,   nullptr, ep,             phaseY + 44,180, 32, SWP_NOZORDER | SWP_NOACTIVATE);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    HDC     memDC = CreateCompatibleDC(hdc);
    HBITMAP bmp   = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old   = (HBITMAP)SelectObject(memDC, bmp);

    const auto& C = Theme::colors();
    FillRectColor(memDC, rc, C.background);

    // ── Section header ────────────────────────────────────────────────────────
    RECT hdrRc{0, 0, rc.right, HDR_H};
    FillRectColor(memDC, hdrRc, C.background);

    RECT hdrText{16, 0, rc.right - 16, HDR_H - 4};
    DrawText_(memDC, L"\u2328  Kisayol Ayarlari", hdrText,
              C.primaryText, 13, true, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Accent separator
    RECT hdrLine{16, HDR_H - 3, rc.right - 16, HDR_H - 1};
    FillRectColor(memDC, hdrLine, C.primaryBlue);

    // ── Shortcut rows ─────────────────────────────────────────────────────────
    for (int i = 0; i < (int)m_shortcuts.size(); ++i) {
        const auto& cfg = m_shortcuts[i];
        RECT row{0, HDR_H + i * ROW_H, rc.right, HDR_H + (i + 1) * ROW_H};

        // Alternate background + selection
        COLORREF rowBg = (i % 2 == 0) ? C.background
                                       : Utils::blendColor(C.background, C.surface, 0.5f);
        if (i == m_selectedIdx)
            rowBg = Utils::blendColor(rowBg, C.primaryBlue, 0.10f);
        FillRectColor(memDC, row, rowBg);

        // Left accent bar for selected
        if (i == m_selectedIdx) {
            RECT accent{0, row.top + 4, 4, row.bottom - 4};
            FillRectColor(memDC, accent, C.primaryBlue);
        }

        // Row bottom border
        RECT border{8, row.bottom - 1, rc.right - 8, row.bottom};
        FillRectColor(memDC, border, C.border);

        // Action name
        RECT namRc{16, row.top + 10, (rc.right / 2), row.top + 32};
        DrawText_(memDC, cfg.actionName, namRc,
                  C.primaryText, 10, true,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

        // Phase description
        RECT phaseRc{16, row.top + 34, (rc.right / 2), row.bottom - 8};
        DrawText_(memDC, describePhase(cfg), phaseRc,
                  C.secondaryText, 9, false,
                  DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

        // Key badge
        std::wstring keyLabel = m_recording && i == m_selectedIdx
            ? L"Bekleniyor..."
            : describeKey(cfg.vkCode, cfg.modifiers);

        COLORREF badgeBg = (m_recording && i == m_selectedIdx)
            ? Utils::blendColor(C.surface, C.primaryBlue, 0.25f)
            : C.surface;

        int badgeW  = 160;
        int badgeH  = 32;
        int badgeX  = rc.right - badgeW - 24;
        int badgeY2 = row.top + (ROW_H - badgeH) / 2;
        RECT badgeRc{badgeX, badgeY2, badgeX + badgeW, badgeY2 + badgeH};
        DrawRoundRect(memDC, badgeRc, 6, badgeBg, C.border, 1);
        DrawText_(memDC, keyLabel, badgeRc,
                  C.primaryText, 9, true,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // ── Edit panel header ─────────────────────────────────────────────────────
    if (m_selectedIdx >= 0) {
        int editPanelY = HDR_H + (int)m_shortcuts.size() * ROW_H + 16;
        RECT subHdr{16, editPanelY - 14, rc.right - 16, editPanelY + 2};
        DrawText_(memDC, L"Secili Kisayolu Duzenle:", subHdr,
                  C.secondaryText, 9, false,
                  DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, old);
    DeleteObject(bmp);
    DeleteDC(memDC);
    EndPaint(m_hwnd, &ps);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::onLButtonDown(int x, int y) {
    int idx = hitTestRow(y);
    if (idx != m_selectedIdx) {
        if (m_recording) cancelRecording();
        m_selectedIdx = idx;
        updateEditPanel();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::updateEditPanel() {
    bool hasSel = (m_selectedIdx >= 0);

    // Show/hide all edit controls
    int sw = hasSel ? SW_SHOW : SW_HIDE;
    if (m_hBtnRecord)  ShowWindow(m_hBtnRecord,  sw);
    if (m_hBtnSave)    ShowWindow(m_hBtnSave,    sw);
    if (m_hBtnCancel)  ShowWindow(m_hBtnCancel,  sw);
    if (m_hPhaseDown)  ShowWindow(m_hPhaseDown,  sw);
    if (m_hPhaseHold)  ShowWindow(m_hPhaseHold,  sw);
    if (m_hPhaseTap)   ShowWindow(m_hPhaseTap,   sw);

    if (hasSel) {
        const auto& cfg = m_shortcuts[m_selectedIdx];

        // Set record button label
        SetWindowTextW(m_hBtnRecord,
            m_recording ? L"\u23F9 Durdur" : L"\u23F4 Kayit Basla");

        // Check correct phase radio
        SendMessageW(m_hPhaseDown, BM_SETCHECK,
            (cfg.keyPhase == L"down" ? BST_CHECKED : BST_UNCHECKED), 0);
        SendMessageW(m_hPhaseHold, BM_SETCHECK,
            (cfg.keyPhase == L"hold" ? BST_CHECKED : BST_UNCHECKED), 0);
        SendMessageW(m_hPhaseTap,  BM_SETCHECK,
            (cfg.keyPhase == L"tap"  ? BST_CHECKED : BST_UNCHECKED), 0);

        // Tap count
        SetWindowTextW(m_hTapCount, std::to_wstring(cfg.tapCount).c_str());
        updatePhaseControls();
    }
}

void SettingsPage::updatePhaseControls() {
    bool isTap = (SendMessageW(m_hPhaseTap, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (m_hTapCount) ShowWindow(m_hTapCount, isTap ? SW_SHOW : SW_HIDE);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::startRecording() {
    if (m_selectedIdx < 0) return;
    m_recording    = true;
    m_recordedVk   = 0;
    m_recordedMods = 0;
    SetWindowTextW(m_hBtnRecord, L"\u23F9 Durdur");
    SetFocus(m_hwnd);   // so WM_KEYDOWN comes to us
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SettingsPage::stopRecording(UINT vk, UINT modifiers) {
    m_recording    = false;
    m_recordedVk   = vk;
    m_recordedMods = modifiers;

    if (m_selectedIdx >= 0 && m_selectedIdx < (int)m_shortcuts.size()) {
        m_shortcuts[m_selectedIdx].vkCode    = vk;
        m_shortcuts[m_selectedIdx].modifiers = modifiers;
    }

    SetWindowTextW(m_hBtnRecord, L"\u23F4 Kayit Basla");
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SettingsPage::cancelRecording() {
    m_recording = false;
    SetWindowTextW(m_hBtnRecord, L"\u23F4 Kayit Basla");
    // Restore original keys from DB
    auto fresh = Database::get().getAllShortcuts();
    if (m_selectedIdx < (int)fresh.size())
        m_shortcuts[m_selectedIdx] = fresh[m_selectedIdx];
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SettingsPage::applyPhaseFromRadios() {
    if (m_selectedIdx < 0 || m_selectedIdx >= (int)m_shortcuts.size()) return;
    auto& cfg = m_shortcuts[m_selectedIdx];
    if (SendMessageW(m_hPhaseHold, BM_GETCHECK, 0, 0) == BST_CHECKED)
        cfg.keyPhase = L"hold";
    else if (SendMessageW(m_hPhaseTap, BM_GETCHECK, 0, 0) == BST_CHECKED)
        cfg.keyPhase = L"tap";
    else
        cfg.keyPhase = L"down";

    if (cfg.keyPhase == L"tap" && m_hTapCount) {
        wchar_t buf[16]{};
        GetWindowTextW(m_hTapCount, buf, 15);
        cfg.tapCount = (_wtoi(buf) > 0) ? _wtoi(buf) : 1;
    }
}

void SettingsPage::saveCurrentEdit() {
    if (m_selectedIdx < 0 || m_selectedIdx >= (int)m_shortcuts.size()) return;
    applyPhaseFromRadios();
    auto& cfg = m_shortcuts[m_selectedIdx];
    Database::get().saveShortcut(cfg);

    // Re-register hotkeys through AppContext
    PostMessageW(AppContext::get().mainWindow, WM_APP + 10, 0, 0);  // WM_RELOAD_SHORTCUTS
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::onKeyDown(UINT vk) {
    if (!m_recording) return;

    // Ignore lone modifier keys (Ctrl, Alt, Shift, Win)
    if (vk == VK_CONTROL || vk == VK_SHIFT || vk == VK_MENU ||
        vk == VK_LCONTROL || vk == VK_RCONTROL ||
        vk == VK_LSHIFT   || vk == VK_RSHIFT   ||
        vk == VK_LMENU    || vk == VK_RMENU    ||
        vk == VK_LWIN     || vk == VK_RWIN)
        return;

    // Collect modifiers from current key state
    UINT mods = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) mods |= MOD_CONTROL;
    if (GetKeyState(VK_MENU)    & 0x8000) mods |= MOD_ALT;
    if (GetKeyState(VK_SHIFT)   & 0x8000) mods |= MOD_SHIFT;

    stopRecording(vk, mods);
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::onCommand(int id, HWND /*hCtrl*/) {
    switch (id) {
    case IDC_BTN_RECORD_KEY:
        if (m_recording) cancelRecording();
        else             startRecording();
        break;

    case IDC_BTN_SAVE_SHORTCUT:
        if (m_recording) cancelRecording();
        saveCurrentEdit();
        break;

    case IDC_BTN_CANCEL_RECORD:
        cancelRecording();
        m_selectedIdx = -1;
        updateEditPanel();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        break;

    case IDC_PHASE_DOWN_RADIO:
    case IDC_PHASE_HOLD_RADIO:
    case IDC_PHASE_TAP_RADIO:
        updatePhaseControls();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        break;

    case IDC_BTN_RESET_SHORTCUTS:
        if (MessageBoxW(m_hwnd,
            L"Tüm kısayollar varsayılan değerlere sıfırlanacak.\nDevam etmek istiyor musunuz?",
            L"Sıfırla", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            Database::get().resetShortcutsToDefault();
            refreshShortcuts();
            m_selectedIdx = -1;
            updateEditPanel();
            PostMessageW(AppContext::get().mainWindow, WM_APP + 10, 0, 0);
        }
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void SettingsPage::refreshShortcuts() {
    m_shortcuts = Database::get().getAllShortcuts();
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

// ─────────────────────────────────────────────────────────────────────────────
LRESULT CALLBACK SettingsPage::WndProc(HWND hwnd, UINT msg,
                                        WPARAM wp, LPARAM lp) {
    SettingsPage* self =
        (SettingsPage*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (SettingsPage*)cs->lpCreateParams;
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

    case WM_LBUTTONDOWN:
        self->onLButtonDown(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_KEYDOWN:
        self->onKeyDown((UINT)wp);
        return 0;

    case WM_COMMAND:
        self->onCommand(LOWORD(wp), (HWND)lp);
        return 0;

    case WM_DATA_UPDATED:
        self->refreshShortcuts();
        return 0;

    case WM_THEME_CHANGED:
        InvalidateRect(hwnd, nullptr, TRUE);
        // Re-apply theme to child controls
        if (self->m_hBtnRecord)  InvalidateRect(self->m_hBtnRecord,  nullptr, TRUE);
        if (self->m_hBtnSave)    InvalidateRect(self->m_hBtnSave,    nullptr, TRUE);
        if (self->m_hBtnCancel)  InvalidateRect(self->m_hBtnCancel,  nullptr, TRUE);
        if (self->m_hBtnReset)   InvalidateRect(self->m_hBtnReset,   nullptr, TRUE);
        return 0;

    // Theme-aware radio/edit backgrounds
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.background);
        static HBRUSH s_br = nullptr;
        if (s_br) DeleteObject(s_br);
        s_br = CreateSolidBrush(C.background);
        return (LRESULT)s_br;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
