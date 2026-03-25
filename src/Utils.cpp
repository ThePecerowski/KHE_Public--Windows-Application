#include "Utils.h"
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <map>
#include <algorithm>
#include <cctype>

namespace Utils {

// ── String conversion ─────────────────────────────────────────────────────────
std::wstring utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int sz = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(sz - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), sz);
    return out;
}

std::string wideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(sz - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), sz, nullptr, nullptr);
    return out;
}

// ── App data dir ──────────────────────────────────────────────────────────────
std::wstring getAppDataDir() {
    wchar_t buf[MAX_PATH] = {};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, buf);
    std::wstring dir = buf;
    dir += L"\\KHE";
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir;
}

// ── File / shell ──────────────────────────────────────────────────────────────
bool openFile(const std::wstring& path) {
    HINSTANCE hr = ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return (INT_PTR)hr > 32;
}

bool showInExplorer(const std::wstring& path) {
    // If path is a file, select it; if a folder, open it
    LPITEMIDLIST pidl = ILCreateFromPathW(path.c_str());
    if (!pidl) {
        // Fallback: open parent folder
        wchar_t parent[MAX_PATH];
        wcsncpy_s(parent, path.c_str(), MAX_PATH);
        PathRemoveFileSpecW(parent);
        return (INT_PTR)ShellExecuteW(nullptr, L"open", parent, nullptr, nullptr, SW_SHOWNORMAL) > 32;
    }
    HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    ILFree(pidl);
    return SUCCEEDED(hr);
}

bool playMedia(const std::wstring& path) {
    return openFile(path);
}

std::wstring browseForFile(HWND parent, const wchar_t* title, const wchar_t* filter) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn   = {};
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = parent;
    ofn.lpstrFile       = buf;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrFilter     = filter;
    ofn.lpstrTitle      = title;
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&ofn))
        return buf;
    return {};
}

std::wstring detectMediaType(const std::wstring& path) {
    // Get lowercase extension
    const wchar_t* ext = PathFindExtensionW(path.c_str());
    if (!ext || *ext == L'\0') return L"file";

    std::wstring e = ext;
    for (auto& c : e) c = (wchar_t)towlower(c);

    static const wchar_t* imgExts[]   = { L".jpg",L".jpeg",L".png",L".gif",L".bmp",L".webp",L".tiff",nullptr };
    static const wchar_t* vidExts[]   = { L".mp4",L".avi",L".mkv",L".mov",L".wmv",L".flv",nullptr };
    static const wchar_t* audExts[]   = { L".mp3",L".wav",L".flac",L".aac",L".ogg",L".wma",nullptr };

    for (int i = 0; imgExts[i]; ++i) if (e == imgExts[i]) return L"image";
    for (int i = 0; vidExts[i]; ++i) if (e == vidExts[i]) return L"video";
    for (int i = 0; audExts[i]; ++i) if (e == audExts[i]) return L"audio";
    return L"file";
}

// ── GDI font cache ────────────────────────────────────────────────────────────
struct FontKey { int size; bool bold; bool operator<(const FontKey& o) const {
    if (size != o.size) return size < o.size; return bold < o.bold; } };

static std::map<FontKey, HFONT> s_fontCache;

HFONT getFont(int pointSize, bool bold) {
    FontKey key{pointSize, bold};
    auto it = s_fontCache.find(key);
    if (it != s_fontCache.end()) return it->second;

    HDC hdc = GetDC(nullptr);
    int height = -MulDiv(pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(nullptr, hdc);

    HFONT f = CreateFontW(height, 0, 0, 0,
        bold ? FW_SEMIBOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
        L"Segoe UI");
    s_fontCache[key] = f;
    return f;
}

// ── Date format ───────────────────────────────────────────────────────────────
std::wstring formatDateTime(const std::wstring& iso) {
    if (iso.size() < 10) return iso;
    // ISO: "YYYY-MM-DD HH:MM:SS"  -> "DD.MM.YYYY HH:MM"
    std::wstring out;
    out += iso.substr(8, 2);   // DD
    out += L'.';
    out += iso.substr(5, 2);   // MM
    out += L'.';
    out += iso.substr(0, 4);   // YYYY
    if (iso.size() >= 16) {
        out += L' ';
        out += iso.substr(11, 5); // HH:MM
    }
    return out;
}

std::wstring truncate(const std::wstring& s, size_t maxLen) {
    if (s.size() <= maxLen) return s;
    return s.substr(0, maxLen - 3) + L"...";
}

} // namespace Utils
