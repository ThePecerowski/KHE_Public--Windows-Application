#pragma once
#include <windows.h>
#include <vector>
#include "Models.h"

class PathsPage {
public:
    bool create(HWND parent, HINSTANCE hi, int x, int y, int w, int h);
    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onCreate(HWND hwnd, int w, int h);
    void onSize(int w, int h);
    void onPaint();
    void onCommand(int id, HWND hCtrl);
    void onNotify(NMHDR* nm);
    void refreshList();

    // Open the AddPathDialog for add or edit
    void openAddDialog(int editId = 0);
    void deleteSelected();

    HWND m_hwnd      = nullptr;
    HWND m_hList     = nullptr;   // SysListView32
    HWND m_hBtnAdd   = nullptr;

    std::vector<PathEntry> m_paths;   // local cache
};
