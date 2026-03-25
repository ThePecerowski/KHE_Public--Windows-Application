#pragma once
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

// ── Note ──────────────────────────────────────────────────────────────────────
struct Note {
    int                     id           = 0;
    std::wstring            title;
    std::string             rtfContent;  // raw RTF bytes (binary-safe std::string)
    std::wstring            lastModified;
    std::wstring            createdAt;
    std::vector<NoteMedia>  media;
};
