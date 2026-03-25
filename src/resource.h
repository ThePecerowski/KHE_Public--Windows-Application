#pragma once

// ── Icons ────────────────────────────────────────────────────────────────────
#define IDI_APP_ICON            101

// ── Menus ────────────────────────────────────────────────────────────────────
#define IDR_MAINMENU            200
#define IDM_FILE_EXIT           40001
#define IDM_VIEW_LIGHT_THEME    40002
#define IDM_VIEW_DARK_THEME     40003
#define IDM_HELP_ABOUT          40004

// ── Sidebar / TopBar ─────────────────────────────────────────────────────────
#define IDC_SIDEBAR             2001
#define IDC_BREADCRUMB          2002
#define IDC_BTN_THEME_TOGGLE    2004

// ── Dashboard lists ───────────────────────────────────────────────────────────
#define IDC_RECENT_PATHS_LIST   2010
#define IDC_RECENT_NOTES_LIST   2011

// ── Paths page controls ───────────────────────────────────────────────────────
#define IDC_PATHS_LIST          5001
#define IDC_BTN_ADD_PATH        5002
#define IDC_BTN_EDIT_PATH       5003
#define IDC_BTN_DELETE_PATH     5004
#define IDC_BTN_OPEN_PATH       5005
#define IDC_BTN_SHOW_PATH       5006

// ── Notes page controls ───────────────────────────────────────────────────────
#define IDC_BTN_ADD_NOTE        6001

// ── Dashboard controls ───────────────────────────────────────────────────────
#define IDC_BTN_QUICK_ADD_PATH  7001
#define IDC_BTN_QUICK_ADD_NOTE  7002

// ── AddPath dialog controls ───────────────────────────────────────────────────
#define IDC_PATH_NAME_EDIT      3001
#define IDC_PATH_PATH_EDIT      3002
#define IDC_PATH_DESC_EDIT      3003
#define IDC_BTN_BROWSE_FILE     3004

// ── NoteEditor dialog controls ────────────────────────────────────────────────
#define IDC_NOTE_TITLE_EDIT     4001
#define IDC_NOTE_RICHTEXT       4002
#define IDC_BTN_FONT_BOLD       4010
#define IDC_BTN_FONT_ITALIC     4011
#define IDC_BTN_FONT_UNDERLINE  4012
#define IDC_BTN_ADD_IMAGE       4013
#define IDC_BTN_ADD_VIDEO       4014
#define IDC_BTN_ADD_AUDIO       4015
#define IDC_MEDIA_LIST          4016

// ── NoteViewer controls ────────────────────────────────────────────────────────
#define IDC_VIEWER_RICHTEXT     8001
#define IDC_VIEWER_MEDIA_LIST   8002
#define IDC_BTN_PLAY_MEDIA      8003
#define IDC_BTN_BACK            8004
#define IDC_BTN_EDIT_NOTE       8005

// ── Custom window messages ────────────────────────────────────────────────────
#define WM_NAVIGATE             (WM_APP + 1)
#define WM_THEME_CHANGED        (WM_APP + 2)
#define WM_DATA_UPDATED         (WM_APP + 3)

// ── Page indices (wParam for WM_NAVIGATE) ─────────────────────────────────────
#define PAGE_DASHBOARD          0
#define PAGE_PATHS              1
#define PAGE_NOTES              2
#define PAGE_NOTE_VIEWER        3
