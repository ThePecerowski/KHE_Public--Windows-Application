#pragma once
#include <windows.h>

class Dashboard {
public:
    bool create(HWND parent, HINSTANCE hi, int x, int y, int w, int h);
    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onCreate(HWND hwnd, int w, int h);
    void onSize(int w, int h);
    void onPaint();
    void onCommand(int id);
    void refreshData();

    HWND m_hwnd       = nullptr;
    HWND m_hPathsList = nullptr;   // simple ListBox for recent paths
    HWND m_hNotesList = nullptr;   // simple ListBox for recent notes
    HWND m_hAddPath   = nullptr;
    HWND m_hAddNote   = nullptr;
};
