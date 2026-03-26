#pragma once
#include <string>
#include <vector>
#include "Models.h"

struct sqlite3;

class Database {
public:
    static Database& get();

    bool initialize(const std::wstring& dbPath);
    void close();

    // ── Path CRUD ─────────────────────────────────────────────────────────────
    bool                   addPath(PathEntry& out);
    bool                   updatePath(const PathEntry& entry);
    bool                   deletePath(int id);
    std::vector<PathEntry> getAllPaths();
    std::vector<PathEntry> getRecentPaths(int count = 5);
    bool                   touchPath(int id);          // update last_used

    // ── Note CRUD ─────────────────────────────────────────────────────────────
    bool               addNote(Note& out);
    bool               updateNote(const Note& note);
    bool               deleteNote(int id);
    std::vector<Note>  getAllNotes();                   // without media/content
    std::vector<Note>  getRecentNotes(int count = 5);
    Note               getNoteById(int id);            // full note + media

    // ── Note media ────────────────────────────────────────────────────────────
    bool                      addNoteMedia(NoteMedia& out);
    bool                      deleteNoteMedia(int id);
    bool                      deleteAllNoteMedia(int noteId);
    std::vector<NoteMedia>    getNoteMedia(int noteId);

    // ── Shortcuts ─────────────────────────────────────────────────────────────
    std::vector<ShortcutConfig> getAllShortcuts();
    bool                        saveShortcut(ShortcutConfig& cfg);   // insert or update
    bool                        resetShortcutsToDefault();           // delete all and re-seed

    ~Database();

private:
    Database()               = default;
    Database(const Database&)= delete;
    Database& operator=(const Database&) = delete;

    bool createTables();
    bool execSQL(const char* sql);

    static std::string  wideToUtf8(const std::wstring& w);
    static std::wstring utf8ToWide(const std::string& s);

    sqlite3* m_db = nullptr;
};
