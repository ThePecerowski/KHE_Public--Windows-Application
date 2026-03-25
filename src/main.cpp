#include <windows.h>
#include <commctrl.h>
#include "MainWindow.h"
#include "Database.h"
#include "Utils.h"
#include "AppContext.h"

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

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Database::get().close();
    return static_cast<int>(msg.wParam);
}
