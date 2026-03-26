#pragma once
#include <windows.h>
#include <vector>
#include "Models.h"

// ── Settings Page ─────────────────────────────────────────────────────────────
// Displays all configurable keyboard shortcuts. Users can click a row to select
// it, then press "Kayıt Başlat" to live-record a new key combination, choose
// the key phase (Bas / Basılı Tut / X Kez Bas) and save. Theme-aware.
class SettingsPage {
public:
    bool create(HWND parent, HINSTANCE hi, int x, int y, int w, int h);
    HWND hwnd() const { return m_hwnd; }

    // Call after shortcuts are changed externally to refresh the list
    void refreshShortcuts();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    void onCreate(HWND hwnd, int w, int h);
    void onSize(int w, int h);
    void onPaint();
    void onCommand(int id, HWND hCtrl);
    void onLButtonDown(int x, int y);
    void onKeyDown(UINT vk);   // while m_recording == true

    // Recording flow
    void startRecording();
    void stopRecording(UINT vk, UINT modifiers);
    void cancelRecording();
    void saveCurrentEdit();
    void applyPhaseFromRadios();

    // Helpers
    void updateEditPanel();         // show/hide edit controls, update labels
    void updatePhaseControls();     // tap-count visibility
    std::wstring describeKey(UINT vk, UINT mods) const;
    std::wstring describePhase(const ShortcutConfig& cfg) const;
    int  hitTestRow(int y) const;   // returns shortcut index or -1

    // ── Child controls (editing panel) ────────────────────────────────────────
    HWND m_hBtnRecord   = nullptr;  // "Kayıt Başlat" / "Bekleniyor..."
    HWND m_hBtnSave     = nullptr;  // "Kaydet"
    HWND m_hBtnCancel   = nullptr;  // "İptal"
    HWND m_hBtnReset    = nullptr;  // "Varsayılana Sıfırla"
    HWND m_hPhaseDown   = nullptr;  // radio: Bas (key-down)
    HWND m_hPhaseHold   = nullptr;  // radio: Basılı Tut
    HWND m_hPhaseTap    = nullptr;  // radio: X Kez Bas
    HWND m_hTapCount    = nullptr;  // edit: tap count number

    // ── State ─────────────────────────────────────────────────────────────────
    HWND m_hwnd         = nullptr;
    std::vector<ShortcutConfig> m_shortcuts;
    int  m_selectedIdx  = -1;   // which row is selected
    bool m_recording    = false;
    UINT m_recordedVk   = 0;
    UINT m_recordedMods = 0;

    // Layout constants
    static constexpr int HDR_H    = 56;   // section header height
    static constexpr int ROW_H    = 68;   // shortcut row height
    static constexpr int EDIT_Y   = 0;    // computed in onSize
    static constexpr int EDIT_PAD = 16;
};
