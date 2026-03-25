#include "NotesPage.h"
#include <algorithm>
#include "AppContext.h"
#include "Theme.h"
#include "Controls.h"
#include "Utils.h"
#include "Database.h"
#include "NoteEditorDialog.h"
#include "resource.h"

static const wchar_t* NOTES_CLASS = L"KHE_NotesPage";

bool NotesPage::create(HWND parent, HINSTANCE hi, int x, int y, int w, int h) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = NOTES_CLASS;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.style         = CS_DBLCLKS;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, NOTES_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, hi, this);
    return m_hwnd != nullptr;
}

void NotesPage::onCreate(HWND hwnd, int w, int /*h*/) {
    m_hwnd = hwnd;
    HINSTANCE hi = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

    m_hBtnAdd = CreateWindowW(L"BUTTON", L"+ Yeni Not Ekle",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        16, 72, 180, 34,
        hwnd, (HMENU)IDC_BTN_ADD_NOTE, hi, nullptr);
    SendMessageW(m_hBtnAdd, WM_SETFONT, (WPARAM)Utils::getFont(10), TRUE);

    m_contentWidth = w;
}

void NotesPage::onSize(int w, int h) {
    m_contentWidth = w;
    SetWindowPos(m_hBtnAdd, nullptr, 16, 72, 180, 34, SWP_NOZORDER);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void NotesPage::refreshData() {
    m_notes = Database::get().getAllNotes();
    m_scrollOffset = 0;
    m_hoverCard    = -1;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

RECT NotesPage::cardRect(int idx) const {
    RECT rc; GetClientRect(m_hwnd, &rc);
    int cols = std::max(1, (int)(rc.right - CARD_MARGIN * 2 + CARD_GAP) / (CARD_W + CARD_GAP));
    int col  = idx % cols;
    int row  = idx / cols;
    int x    = CARD_MARGIN + col * (CARD_W + CARD_GAP);
    int y    = 120 + row * (CARD_H + CARD_GAP) - m_scrollOffset;
    return RECT{x, y, x + CARD_W, y + CARD_H};
}

int NotesPage::hitTestCard(int mx, int my) const {
    for (int i = 0; i < (int)m_notes.size(); ++i) {
        RECT r = cardRect(i);
        if (mx >= r.left && mx < r.right && my >= r.top && my < r.bottom)
            return i;
    }
    return -1;
}

void NotesPage::onPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT rc; GetClientRect(m_hwnd, &rc);

    HDC    mem  = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    const auto& C = Theme::colors();
    FillRectColor(mem, rc, C.background);

    // Title
    RECT titleRc{16, 16, rc.right - 16, 56};
    DrawText_(mem, L"Notlar", titleRc, C.primaryText, 20, true,
              DT_LEFT | DT_TOP | DT_SINGLELINE);
    RECT subtRc{16, 50, rc.right - 16, 72};
    DrawText_(mem, L"Rich-text notlarınızı yönetin", subtRc,
              C.secondaryText, 9, false, DT_LEFT | DT_TOP | DT_SINGLELINE);

    if (m_notes.empty()) {
        RECT emptyRc{0, 160, rc.right, 220};
        DrawText_(mem, L"Henüz not eklenmedi.", emptyRc, C.secondaryText,
                  11, false, DT_CENTER | DT_TOP | DT_SINGLELINE);
    }

    // Draw cards
    for (int i = 0; i < (int)m_notes.size(); ++i) {
        RECT cr = cardRect(i);
        if (cr.bottom < 0 || cr.top > rc.bottom) continue;

        bool hover = (i == m_hoverCard);
        COLORREF cardBg     = hover ? Utils::blendColor(C.card, C.primaryBlue, 0.08f) : C.card;
        COLORREF borderClr  = hover ? C.primaryBlue : C.border;

        DrawRoundRect(mem, cr, 10, cardBg, borderClr, hover ? 2 : 1);

        // Title
        RECT tRc{cr.left + 12, cr.top + 12, cr.right - 12, cr.top + 38};
        DrawText_(mem, Utils::truncate(m_notes[i].title, 32), tRc,
                  C.primaryText, 11, true,
                  DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

        // Date
        RECT dRc{cr.left + 12, cr.top + 40, cr.right - 12, cr.top + 60};
        std::wstring dateStr = L"✎  " + Utils::formatDateTime(m_notes[i].lastModified);
        DrawText_(mem, dateStr, dRc, C.secondaryText, 8, false,
                  DT_LEFT | DT_TOP | DT_SINGLELINE);

        // Action hints at bottom of card
        RECT actRc{cr.left + 8, cr.bottom - 30, cr.right - 8, cr.bottom - 6};
        // Divider
        RECT divRc{cr.left + 8, cr.bottom - 34, cr.right - 8, cr.bottom - 33};
        FillRectColor(mem, divRc, C.border);
        DrawText_(mem, L"Düzenle   Sil", actRc, C.secondaryText, 8, false,
                  DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old); DeleteObject(bmp); DeleteDC(mem);
    EndPaint(m_hwnd, &ps);
}

void NotesPage::onMouseMove(int x, int y) {
    if (!m_tracking) {
        TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, m_hwnd, 0};
        TrackMouseEvent(&tme);
        m_tracking = true;
    }
    int idx = hitTestCard(x, y);
    if (idx != m_hoverCard) {
        m_hoverCard = idx;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void NotesPage::onMouseLeave() {
    m_tracking  = false;
    m_hoverCard = -1;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void NotesPage::onLButtonDown(int x, int y) {
    int idx = hitTestCard(x, y);
    if (idx < 0) return;

    // Check if click is in the bottom "Düzenle / Sil" area
    RECT cr = cardRect(idx);
    if (y >= cr.bottom - 34 && y < cr.bottom) {
        // Left half = Edit, Right half = Delete
        int mid = (cr.left + cr.right) / 2;
        if (x < mid)
            openEditor(m_notes[idx].id);
        else
            deleteNote(idx);
    } else {
        // Click on card body → view note
        AppContext::get().viewingNoteId = m_notes[idx].id;
        AppContext::get().navigateTo(PAGE_NOTE_VIEWER);
    }
}

void NotesPage::openEditor(int noteId) {
    NoteEditorDialog dlg;
    Note note{};
    if (noteId > 0) note = Database::get().getNoteById(noteId);
    if (dlg.show(m_hwnd, note)) {
        if (note.id > 0) {
            Database::get().updateNote(note);
            // Re-sync media: delete all existing, then re-insert from the in-memory list
            Database::get().deleteAllNoteMedia(note.id);
            for (auto& med : note.media) {
                med.noteId = note.id;
                Database::get().addNoteMedia(med);
            }
        } else {
            Database::get().addNote(note);   // sets note.id
            for (auto& med : note.media) {
                med.noteId = note.id;
                Database::get().addNoteMedia(med);
            }
        }
        refreshData();
    }
}

void NotesPage::deleteNote(int idx) {
    if (idx < 0 || idx >= (int)m_notes.size()) return;
    if (MessageBoxW(m_hwnd,
        L"Bu notu silmek istediğinize emin misiniz?",
        L"Sil", MB_YESNO | MB_ICONWARNING) == IDYES) {
        Database::get().deleteAllNoteMedia(m_notes[idx].id);
        Database::get().deleteNote(m_notes[idx].id);
        refreshData();
    }
}

void NotesPage::onCommand(int id) {
    if (id == IDC_BTN_ADD_NOTE)
        openEditor(0);
}

LRESULT CALLBACK NotesPage::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    NotesPage* self = (NotesPage*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        self = (NotesPage*)cs->lpCreateParams;
        self->m_hwnd = hwnd;
    }
    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case WM_CREATE: {
        RECT rc; GetClientRect(hwnd, &rc);
        self->onCreate(hwnd, rc.right, rc.bottom);
        return 0;
    }
    case WM_SIZE:        self->onSize(LOWORD(lp), HIWORD(lp));  return 0;
    case WM_PAINT:       self->onPaint();                        return 0;
    case WM_ERASEBKGND:  return 1;
    case WM_COMMAND:     self->onCommand(LOWORD(wp));            return 0;
    case WM_LBUTTONDOWN: self->onLButtonDown(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_MOUSEMOVE:   self->onMouseMove(LOWORD(lp), HIWORD(lp)); return 0;
    case WM_MOUSELEAVE:  self->onMouseLeave();                   return 0;
    case WM_DATA_UPDATED: self->refreshData();                   return 0;
    case WM_THEME_CHANGED: InvalidateRect(hwnd, nullptr, TRUE); return 0;
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wp);
        self->m_scrollOffset -= delta / 3;
        if (self->m_scrollOffset < 0) self->m_scrollOffset = 0;
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        const auto& C = Theme::colors();
        SetTextColor(hdc, C.primaryText);
        SetBkColor(hdc, C.background);
        static HBRUSH s_br = nullptr;
        if (s_br) DeleteObject(s_br);
        s_br = CreateSolidBrush(C.background);
        return (LRESULT)s_br;
    }
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
