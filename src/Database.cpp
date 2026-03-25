#include "Database.h"
#include <sqlite3.h>
#include <windows.h>
#include <sstream>
#include <cassert>

// ── Singleton ─────────────────────────────────────────────────────────────────
Database& Database::get() {
    static Database instance;
    return instance;
}

Database::~Database() { close(); }

void Database::close() {
    if (m_db) { sqlite3_close(m_db); m_db = nullptr; }
}

// ── String helpers ────────────────────────────────────────────────────────────
std::string Database::wideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int sz = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(sz - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), sz, nullptr, nullptr);
    return out;
}

std::wstring Database::utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int sz = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring out(sz - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), sz);
    return out;
}

// ── Init ──────────────────────────────────────────────────────────────────────
bool Database::initialize(const std::wstring& dbPath) {
    std::string utf8 = wideToUtf8(dbPath);
    if (sqlite3_open(utf8.c_str(), &m_db) != SQLITE_OK) {
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }
    sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);
    return createTables();
}

bool Database::execSQL(const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool Database::createTables() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS paths ("
        "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name         TEXT    NOT NULL,"
        "  path         TEXT    NOT NULL,"
        "  description  TEXT,"
        "  last_used    TEXT,"
        "  created_at   TEXT DEFAULT (datetime('now'))"
        ");"

        "CREATE TABLE IF NOT EXISTS notes ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title         TEXT NOT NULL,"
        "  content       BLOB,"
        "  last_modified TEXT,"
        "  created_at    TEXT DEFAULT (datetime('now'))"
        ");"

        "CREATE TABLE IF NOT EXISTS note_media ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  note_id     INTEGER NOT NULL,"
        "  media_path  TEXT    NOT NULL,"
        "  media_type  TEXT,"
        "  FOREIGN KEY(note_id) REFERENCES notes(id) ON DELETE CASCADE"
        ");";
    return execSQL(sql);
}

// ── Paths ─────────────────────────────────────────────────────────────────────
bool Database::addPath(PathEntry& out) {
    const char* sql =
        "INSERT INTO paths(name,path,description) VALUES(?,?,?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    auto n = wideToUtf8(out.name);
    auto p = wideToUtf8(out.path);
    auto d = wideToUtf8(out.description);
    sqlite3_bind_text(stmt, 1, n.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, d.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok) out.id = (int)sqlite3_last_insert_rowid(m_db);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updatePath(const PathEntry& e) {
    const char* sql =
        "UPDATE paths SET name=?,path=?,description=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    auto n = wideToUtf8(e.name);
    auto p = wideToUtf8(e.path);
    auto d = wideToUtf8(e.description);
    sqlite3_bind_text(stmt, 1, n.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, d.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, e.id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::deletePath(int id) {
    const char* sql = "DELETE FROM paths WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_int(stmt, 1, id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

static PathEntry rowToPath(sqlite3_stmt* s) {
    PathEntry e;
    e.id          = sqlite3_column_int(s, 0);
    auto col = [&](int i) -> std::string {
        auto* t = reinterpret_cast<const char*>(sqlite3_column_text(s, i));
        return t ? t : "";
    };
    auto uw = [](const std::string& u) -> std::wstring {
        if (u.empty()) return {};
        int sz = MultiByteToWideChar(CP_UTF8, 0, u.c_str(), -1, nullptr, 0);
        std::wstring out(sz - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, u.c_str(), -1, out.data(), sz);
        return out;
    };
    e.name        = uw(col(1));
    e.path        = uw(col(2));
    e.description = uw(col(3));
    e.lastUsed    = uw(col(4));
    e.createdAt   = uw(col(5));
    return e;
}

std::vector<PathEntry> Database::getAllPaths() {
    std::vector<PathEntry> result;
    const char* sql =
        "SELECT id,name,path,description,last_used,created_at FROM paths ORDER BY name";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        result.push_back(rowToPath(stmt));
    sqlite3_finalize(stmt);
    return result;
}

std::vector<PathEntry> Database::getRecentPaths(int count) {
    std::vector<PathEntry> result;
    const char* sql =
        "SELECT id,name,path,description,last_used,created_at FROM paths "
        "ORDER BY last_used DESC LIMIT ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    sqlite3_bind_int(stmt, 1, count);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        result.push_back(rowToPath(stmt));
    sqlite3_finalize(stmt);
    return result;
}

bool Database::touchPath(int id) {
    const char* sql =
        "UPDATE paths SET last_used=datetime('now') WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_int(stmt, 1, id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// ── Notes ─────────────────────────────────────────────────────────────────────
bool Database::addNote(Note& out) {
    const char* sql =
        "INSERT INTO notes(title,content,last_modified) VALUES(?,?,datetime('now'))";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    auto t = wideToUtf8(out.title);
    sqlite3_bind_text(stmt, 1, t.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 2,
        out.rtfContent.data(), (int)out.rtfContent.size(), SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok) out.id = (int)sqlite3_last_insert_rowid(m_db);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateNote(const Note& note) {
    const char* sql =
        "UPDATE notes SET title=?,content=?,last_modified=datetime('now') WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    auto t = wideToUtf8(note.title);
    sqlite3_bind_text(stmt, 1, t.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 2,
        note.rtfContent.data(), (int)note.rtfContent.size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, note.id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::deleteNote(int id) {
    const char* sql = "DELETE FROM notes WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_int(stmt, 1, id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<Note> Database::getAllNotes() {
    std::vector<Note> result;
    const char* sql =
        "SELECT id,title,last_modified,created_at FROM notes ORDER BY last_modified DESC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Note n;
        n.id = sqlite3_column_int(stmt, 0);
        auto col = [&](int i) {
            auto* t = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            return t ? std::string(t) : std::string{};
        };
        n.title        = utf8ToWide(col(1));
        n.lastModified = utf8ToWide(col(2));
        n.createdAt    = utf8ToWide(col(3));
        result.push_back(std::move(n));
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Note> Database::getRecentNotes(int count) {
    std::vector<Note> result;
    const char* sql =
        "SELECT id,title,last_modified,created_at FROM notes "
        "ORDER BY last_modified DESC LIMIT ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    sqlite3_bind_int(stmt, 1, count);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Note n;
        n.id = sqlite3_column_int(stmt, 0);
        auto col = [&](int i) {
            auto* t = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            return t ? std::string(t) : std::string{};
        };
        n.title        = utf8ToWide(col(1));
        n.lastModified = utf8ToWide(col(2));
        n.createdAt    = utf8ToWide(col(3));
        result.push_back(std::move(n));
    }
    sqlite3_finalize(stmt);
    return result;
}

Note Database::getNoteById(int id) {
    Note n;
    const char* sql =
        "SELECT id,title,content,last_modified,created_at FROM notes WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return n;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        n.id = sqlite3_column_int(stmt, 0);
        auto col = [&](int i) {
            auto* t = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            return t ? std::string(t) : std::string{};
        };
        n.title = utf8ToWide(col(1));
        // RTF blob
        const void* blob = sqlite3_column_blob(stmt, 2);
        int blobSz       = sqlite3_column_bytes(stmt, 2);
        if (blob && blobSz > 0)
            n.rtfContent.assign(reinterpret_cast<const char*>(blob), blobSz);
        n.lastModified = utf8ToWide(col(3));
        n.createdAt    = utf8ToWide(col(4));
    }
    sqlite3_finalize(stmt);
    n.media = getNoteMedia(id);
    return n;
}

// ── Note media ────────────────────────────────────────────────────────────────
bool Database::addNoteMedia(NoteMedia& out) {
    const char* sql =
        "INSERT INTO note_media(note_id,media_path,media_type) VALUES(?,?,?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    auto p = wideToUtf8(out.mediaPath);
    auto t = wideToUtf8(out.mediaType);
    sqlite3_bind_int (stmt, 1, out.noteId);
    sqlite3_bind_text(stmt, 2, p.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, t.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok) out.id = (int)sqlite3_last_insert_rowid(m_db);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::deleteNoteMedia(int id) {
    const char* sql = "DELETE FROM note_media WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_int(stmt, 1, id);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::deleteAllNoteMedia(int noteId) {
    const char* sql = "DELETE FROM note_media WHERE note_id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_bind_int(stmt, 1, noteId);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<NoteMedia> Database::getNoteMedia(int noteId) {
    std::vector<NoteMedia> result;
    const char* sql =
        "SELECT id,note_id,media_path,media_type FROM note_media WHERE note_id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    sqlite3_bind_int(stmt, 1, noteId);
    auto uw = [](const char* s) -> std::wstring {
        if (!s) return {};
        int sz = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
        std::wstring out(sz - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s, -1, out.data(), sz);
        return out;
    };
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NoteMedia m;
        m.id        = sqlite3_column_int(stmt, 0);
        m.noteId    = sqlite3_column_int(stmt, 1);
        m.mediaPath = uw(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        m.mediaType = uw(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        result.push_back(m);
    }
    sqlite3_finalize(stmt);
    return result;
}
