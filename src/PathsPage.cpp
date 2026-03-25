#include "PathsPage.h"
#include "AppContext.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "Database.h"
#include "AddPathDialog.h"
#include "resource.h"
#include <commctrl.h>
#include <string>

static const wchar_t* PATHS_CLASS = L"KHE_PathsPage";

// ListView column indices
enum PathCol { COL_NAME=0, COL_PATH, COL_DESC, COL_USED, COL_COUNT };

bool PathsPage::create(HWND parent, HINSTANCE hi, int x, int y, int w, int h) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = PATHS_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, PATHS_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void PathsPage::onCreate(HWND hwnd, int w, int h) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    // "Add Path" button
    m_hBtnAdd = CreateWindowW(L"BUTTON", L"+ Yeni Dosya Yolu Ekle",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        16, 72, 220, 34,
        hwnd, (HMENU)IDC_BTN_ADD_PATH, hi, nullptr);
    SendMessageW(m_hBtnAdd, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    // ListView
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        LVS_REPORT | LVS_SHOWSELALWAYS,
        16, 120, w - 32, h - 136,
        hwnd, (HMENU)IDC_PATHS_LIST, hi, nullptr);
    ListView_SetExtendedListViewStyle(m_hList,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
    SendMessageW(m_hList, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    // Columns
    LVCOLUMNW col{};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    const wchar_t* headers[] = { L"Ad", L"Yol", L"Açıklama", L"Son Kullanım" };
    int widths[] = { 140, 340, 200, 130 };
    for (int i = 0; i < COL_COUNT; ++i) {
        col.cx      = widths[i];
        col.pszText = (LPWSTR)headers[i];
        col.iSubItem = i;
        ListView_InsertColumn(m_hList, i, &col);
    }
}

void PathsPage::onSize(int w, int h) {
    if (!m_hList) return;
    SetWindowPos(m_hBtnAdd, nullptr, 16, 72, 220, 34, SWP_NOZORDER);
    SetWindowPos(m_hList,   nullptr, 16, 120, w - 32, h - 136, SWP_NOZORDER);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void PathsPage::refreshList() {
    m_paths = Database::get().getAllPaths();
    ListView_DeleteAllItems(m_hList);

    for (int i = 0; i < (int)m_paths.size(); ++i) {
        const auto& p = m_paths[i];
        LVITEMW item{};
        item.mask    = LVIF_TEXT | LVIF_PARAM;
        item.iItem   = i;
        item.pszText = (LPWSTR)p.name.c_str();
        item.lParam  = (LPARAM)p.id;
        ListView_InsertItem(m_hList, &item);
        ListView_SetItemText(m_hList, i, COL_PATH, (LPWSTR)p.path.c_str());
        ListView_SetItemText(m_hList, i, COL_DESC, (LPWSTR)p.description.c_str());
        std::wstring used = p.lastUsed.empty() ? L"-" : Utils::formatDateTime(p.lastUsed);
        ListView_SetItemText(m_hList, i, COL_USED, (LPWSTR)used.c_str());
    }
}

void PathsPage::openAddDialog(int editId) {
    AddPathDialog dlg;
    PathEntry entry{};
    if (editId > 0) {
        for (auto& p : m_paths)
            if (p.id == editId) { entry = p; break; }
    }
    if (dlg.show(m_hwnd, entry)) {
        if (editId > 0)
            Database::get().updatePath(entry);
        else
            Database::get().addPath(entry);
        refreshList();
    }
}

void PathsPage::deleteSelected() {
    int idx = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (idx < 0) return;
    LVITEMW item{};
    item.mask   = LVIF_PARAM;
    item.iItem  = idx;
    ListView_GetItem(m_hList, &item);
    int id = (int)item.lParam;

    if (MessageBoxW(m_hwnd,
        L"Bu dosya yolunu silmek istediğinize emin misiniz?",
        L"Sil", MB_YESNO | MB_ICONWARNING) == IDYES) {
        Database::get().deletePath(id);
        refreshList();
    }
}

void PathsPage::onCommand(int id, HWND /*hCtrl*/) {
    switch (id) {
    case IDC_BTN_ADD_PATH:
        openAddDialog(0);
        break;
    case IDC_BTN_EDIT_PATH: {
        int idx = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
        if (idx >= 0) {
            LVITEMW item{}; item.mask = LVIF_PARAM; item.iItem = idx;
            ListView_GetItem(m_hList, &item);
            openAddDialog((int)item.lParam);
        }
        break;
    }
    case IDC_BTN_DELETE_PATH:
        deleteSelected();
        break;
    case IDC_BTN_OPEN_PATH: {
        // Open the file/program directly
        int idx = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
        if (idx < 0) return;
        LVITEMW item{}; item.mask = LVIF_PARAM; item.iItem = idx;
        ListView_GetItem(m_hList, &item);
        int pid = (int)item.lParam;
        for (auto& p : m_paths) {
            if (p.id == pid) {
                Utils::openFile(p.path);
                Database::get().touchPath(pid);
                refreshList();
                break;
            }
        }
        break;
    }
    case IDC_BTN_SHOW_PATH: {
        // Show folder in Explorer
        int idx = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
        if (idx < 0) return;
        LVITEMW item{}; item.mask = LVIF_PARAM; item.iItem = idx;
        ListView_GetItem(m_hList, &item);
        int pid = (int)item.lParam;
        for (auto& p : m_paths) {
            if (p.id == pid) {
                Utils::showInExplorer(p.path);
                Database::get().touchPath(pid);
                refreshList();
                break;
            }
        }
        break;
    }
    }
}

void PathsPage::onNotify(NMHDR* nm) {
    if (nm->hwndFrom != m_hList) return;

    if (nm->code == NM_DBLCLK) {
        // Double-click on path text → show in explorer
        auto* info = reinterpret_cast<NMITEMACTIVATE*>(nm);
        if (info->iItem < 0 || info->iItem >= (int)m_paths.size()) return;
        auto& p = m_paths[info->iItem];
        Utils::showInExplorer(p.path);
        Database::get().touchPath(p.id);
        refreshList();
    }

    if (nm->code == NM_RCLICK) {
        // Right-click context menu
        POINT pt; GetCursorPos(&pt);
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, IDC_BTN_SHOW_PATH, L"Klasörde Göster");
        AppendMenuW(hMenu, MF_STRING, IDC_BTN_OPEN_PATH, L"Dosyayı/Programı Aç");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, IDC_BTN_EDIT_PATH, L"Düzenle");
        AppendMenuW(hMenu, MF_STRING, IDC_BTN_DELETE_PATH, L"Sil");
        int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                 pt.x, pt.y, 0, m_hwnd, nullptr);
        DestroyMenu(hMenu);
        if (cmd) onCommand(cmd, nullptr);
    }
}

void PathsPage::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT rc; GetClientRect(m_hwnd, &rc);

    HDC    mem  = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    const auto& C = Theme::colors();
    FillRectColor(mem, rc, C.background);

    RECT titleRc{16, 16, rc.right - 16, 56};
    DrawText_(mem, L"Dosya Yolları", titleRc, C.primaryText, 20, true,
              DT_LEFT | DT_TOP | DT_SINGLELINE);
    RECT subtRc{16, 50, rc.right - 16, 72};
    DrawText_(mem, L"Kayıtlı dosya ve program yollarınızı yönetin",
              subtRc, C.secondaryText, 9, false, DT_LEFT | DT_TOP | DT_SINGLELINE);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem);
    EndPaint(m_hwnd, &ps);
}

LRESULT CALLBACK PathsPage::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    PathsPage* self = (PathsPage*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (PathsPage*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE: {
        RECT rc; GetClientRect(hwnd, &rc);
        self->onCreate(hwnd, rc.right, rc.bottom);
        return 0;
    }
    case WM_SIZE:    self->onSize(LOWORD(lp), HIWORD(lp));  return 0;
    case WM_PAINT:   self->onPaint();                        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_COMMAND: self->onCommand(LOWORD(wp), (HWND)lp); return 0;
    case WM_NOTIFY:  self->onNotify((NMHDR*)lp);             return 0;
    case WM_DATA_UPDATED: self->refreshList();               return 0;
    case WM_THEME_CHANGED:
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.surface);
        static HBRUSH s_br = nullptr;
        if (s_br) DeleteObject(s_br);
        s_br = CreateSolidBrush(C.surface);
        return (LRESULT)s_br;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.background);
        static HBRUSH s_btnBr = nullptr;
        if (s_btnBr) DeleteObject(s_btnBr);
        s_btnBr = CreateSolidBrush(C.background);
        return (LRESULT)s_btnBr;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
