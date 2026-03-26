#pragma once
#include <windows.h>
#include <string>
#include <vector>

// ── Path Entry ────────────────────────────────────────────────────────────────
struct PathEntry {
    int          id          = 0;
    std::wstring name;
    std::wstring path;
    std::wstring description;
    std::wstring lastUsed;
    std::wstring createdAt;
};

// ── Note Media attachment ─────────────────────────────────────────────────────
struct NoteMedia {
    int          id        = 0;
    int          noteId    = 0;
    std::wstring mediaPath;
    std::wstring mediaType;   // "image" | "video" | "audio"
};

// ── Shortcut Config ──────────────────────────────────────────────────────────
struct ShortcutConfig {
    int          id        = 0;
    std::wstring actionId;       // "quick_launch" | "new_note" | "new_path" | "search" | "tab_switch"
    std::wstring actionName;     // Human-readable Turkish label
    UINT         vkCode    = 0;  // Virtual key code
    UINT         modifiers = 0;  // MOD_CONTROL | MOD_ALT | MOD_SHIFT
    std::wstring keyPhase;       // L"down" | L"hold" | L"tap"
    int          tapCount  = 1;  // relevant when keyPhase == L"tap"
};

// ── Note ──────────────────────────────────────────────────────────────────────
struct Note {
    int                     id           = 0;
    std::wstring            title;
    std::string             rtfContent;  // raw RTF bytes (binary-safe std::string)
    std::wstring            lastModified;
    std::wstring            createdAt;
    std::vector<NoteMedia>  media;
};
