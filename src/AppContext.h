#pragma once
#include <windows.h>
#include <vector>
#include "Models.h"

// ── Global application state ─────────────────────────────────────────────────
class AppContext {
public:
    static AppContext& get() { static AppContext ctx; return ctx; }

    HWND mainWindow    = nullptr;
    HWND hSidebar      = nullptr;
    HWND hTopBar       = nullptr;
    HWND hDashboard    = nullptr;
    HWND hPathsPage    = nullptr;
    HWND hNotesPage    = nullptr;
    HWND hNoteViewer   = nullptr;
    HWND hSettingsPage = nullptr;  // Settings / shortcut config page
    HWND hQuickLaunch  = nullptr;  // Floating quick-launch panel

    int  currentPage   = 0;    // PAGE_* constants
    int  viewingNoteId = -1;   // Note being viewed/edited

    HINSTANCE hInstance = nullptr;

    // Loaded shortcut configs (refreshed from DB on startup and after save)
    std::vector<ShortcutConfig> shortcuts;

    // Navigate to a page (sends WM_NAVIGATE to mainWindow)
    void navigateTo(int page);

    // Broadcast WM_THEME_CHANGED to all page windows
    void broadcastThemeChange();

    // Broadcast WM_DATA_UPDATED to the current page
    void broadcastDataUpdate();

private:
    AppContext() = default;
};
