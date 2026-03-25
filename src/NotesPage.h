#pragma once
#include <windows.h>
#include <vector>
#include "Models.h"

// Card dimensions
static constexpr int CARD_W      = 260;
static constexpr int CARD_H      = 130;
static constexpr int CARD_GAP    = 16;
static constexpr int CARD_MARGIN = 16;

class NotesPage {
public:
    bool create(HWND parent, HINSTANCE hi, int x, int y, int w, int h);
    HWND hwnd() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void onCreate(HWND hwnd, int w, int h);
    void onSize(int w, int h);
    void onPaint();
    void onCommand(int id);
    void onLButtonDown(int x, int y);
    void onMouseMove(int x, int y);
    void onMouseLeave();
    void refreshData();

    // Returns note index from click position, or -1
    int  hitTestCard(int x, int y) const;
    RECT cardRect(int idx) const;

    void openEditor(int noteId = 0);   // 0 = new note
    void deleteNote(int idx);

    HWND              m_hwnd       = nullptr;
    HWND              m_hBtnAdd    = nullptr;
    HWND              m_hScroll    = nullptr;
    std::vector<Note> m_notes;

    int  m_scrollOffset = 0;
    int  m_hoverCard    = -1;
    int  m_contentWidth = 0;
    bool m_tracking     = false;
};
