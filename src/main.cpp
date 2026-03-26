#include <windows.h>
#include <commctrl.h>
#include "MainWindow.h"
#include "Database.h"
#include "Utils.h"
#include "AppContext.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // Enable modern common controls (v6)
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    // Load RichEdit 4.1
    LoadLibraryW(L"MSFTEDIT.DLL");

    AppContext::get().hInstance = hInstance;

    // Initialize database in %APPDATA%/KHE/
    std::wstring dbPath = Utils::getAppDataDir() + L"\\khe_data.db";
    if (!Database::get().initialize(dbPath)) {
        MessageBoxW(nullptr, L"Veritabanı başlatılamadı.", L"KHE Hata", MB_ICONERROR);
        return 1;
    }

    // Create & show main window
    MainWindow mainWnd;
    if (!mainWnd.create(hInstance, nCmdShow)) {
        MessageBoxW(nullptr, L"Ana pencere oluşturulamadı.", L"KHE Hata", MB_ICONERROR);
        return 1;
    }

    // Build accelerator table for in-app keyboard shortcuts.
    // These are configurable via SettingsPage; defaults are Ctrl+N / Ctrl+L /
    // Ctrl+Tab. Ctrl+F reserved for future search (currently no search bar).
    ACCEL accels[] = {
        { FCONTROL | FVIRTKEY, 'N',     IDM_NEW_NOTE_ACCEL  },
        { FCONTROL | FVIRTKEY, 'L',     IDM_NEW_PATH_ACCEL  },
        { FCONTROL | FVIRTKEY, VK_TAB,  IDM_TAB_NEXT_ACCEL  },
    };
    HACCEL hAccel = CreateAcceleratorTable(accels, ARRAYSIZE(accels));

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        // Try in-app accelerators first (Ctrl+N, Ctrl+L, Ctrl+Tab)
        if (!TranslateAccelerator(AppContext::get().mainWindow, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    DestroyAcceleratorTable(hAccel);
    Database::get().close();
    return static_cast<int>(msg.wParam);
}
