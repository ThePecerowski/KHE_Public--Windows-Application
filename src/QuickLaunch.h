#pragma once
#include <windows.h>
#include <string>
#include <vector>

// ── Search result item ────────────────────────────────────────────────────────
struct QuickLaunchResult {
    enum class Type { Note, Path } type = Type::Note;
    int          id = 0;
    std::wstring name;
    std::wstring secondary;  // file path (for Path) or date (for Note)
    std::wstring openTarget; // actual path/id string used to open
};

// ── Global Quick Launch Panel ─────────────────────────────────────────────────
// Floating search panel activated by global hotkey (Alt+Space or Ctrl+Shift+K).
// Usage: call create() once at startup, show()/hide() on hotkey events.
class QuickLaunch {
public:
    bool create(HINSTANCE hi);
    void show();     // center on screen, focus search box
    void hide();
    bool isVisible() const;
    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK SearchEditProc(HWND, UINT, WPARAM, LPARAM,
                                           UINT_PTR, DWORD_PTR);

    void onCreate(HWND hwnd);
    void onPaint();
    void onSearchChanged();
    void onKeyDown(UINT vk);
    void performSearch(const std::wstring& query);
    void openSelected();
    void resizePanel();

    HWND m_hwnd        = nullptr;
    HWND m_hSearchEdit = nullptr;
    std::vector<QuickLaunchResult> m_results;
    int  m_selectedIdx = 0;

    static constexpr int PANEL_W     = 620;
    static constexpr int SEARCH_H    = 52;
    static constexpr int ITEM_H      = 52;
    static constexpr int MAX_VISIBLE = 7;
    static constexpr int HINT_H      = 56;
};
