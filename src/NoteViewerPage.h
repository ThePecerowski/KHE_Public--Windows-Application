#pragma once
#include <windows.h>
#include "Models.h"

class NoteViewerPage {
public:
    bool create(HWND parent, HINSTANCE hi, int x, int y, int w, int h);
    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onCreate(HWND hwnd, int w, int h);
    void onSize(int w, int h);
    void onPaint();
    void onCommand(int id);
    void loadNote(int noteId);

    HWND m_hwnd      = nullptr;
    HWND m_hRichEdit = nullptr;
    HWND m_hMediaList = nullptr;
    HWND m_hBtnBack  = nullptr;
    HWND m_hBtnEdit  = nullptr;
    HWND m_hBtnPlay  = nullptr;

    Note m_note;
};
