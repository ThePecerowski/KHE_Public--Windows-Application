#pragma once
#include <windows.h>
#include <string>

namespace Utils {

    // ── String conversion ─────────────────────────────────────────────────────
    std::wstring  utf8ToWide(const std::string& s);
    std::string   wideToUtf8(const std::wstring& w);

    // ── App data path ─────────────────────────────────────────────────────────
    // Returns %APPDATA%\KHE\ (creates directory if missing)
    std::wstring getAppDataDir();

    // ── File/shell operations ─────────────────────────────────────────────────
    // Open a file or program with its default association
    bool openFile(const std::wstring& path);

    // Show the file selected in its parent folder in Explorer
    bool showInExplorer(const std::wstring& path);

    // Open a Browse-for-file dialog; returns selected path or empty string
    std::wstring browseForFile(HWND parent,
                               const wchar_t* title     = L"Dosya Seç",
                               const wchar_t* filter    = L"Tüm Dosyalar\0*.*\0");

    // Detect media type from file extension
    // returns "image", "video", "audio", or "file"
    std::wstring detectMediaType(const std::wstring& path);

    // Play a media file with its default player
    bool playMedia(const std::wstring& path);

    // ── GDI helpers ───────────────────────────────────────────────────────────
    // Create a Segoe UI font (cached per size/bold)
    HFONT getFont(int pointSize, bool bold = false);

    // Colour helper
    inline COLORREF blendColor(COLORREF a, COLORREF b, float t) {
        int r = (int)(GetRValue(a) + t * (GetRValue(b) - GetRValue(a)));
        int g = (int)(GetGValue(a) + t * (GetGValue(b) - GetGValue(a)));
        int bl= (int)(GetBValue(a) + t * (GetBValue(b) - GetBValue(a)));
        return RGB(r, g, bl);
    }

    // Format a datetime string stored as ISO "YYYY-MM-DD HH:MM:SS"
    std::wstring formatDateTime(const std::wstring& iso);

    // Truncate a wide string to maxLen chars, adding "..." if needed
    std::wstring truncate(const std::wstring& s, size_t maxLen);
}
