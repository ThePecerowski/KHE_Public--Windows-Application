cmake_minimum_required(VERSION 3.20)
project(KHE VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── SQLite3 via FetchContent ──────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    sqlite3_src
    URL      https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite3_src)

add_library(sqlite3 STATIC "${sqlite3_src_SOURCE_DIR}/sqlite3.c")
target_include_directories(sqlite3 PUBLIC "${sqlite3_src_SOURCE_DIR}")
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
)

# ── Application sources ───────────────────────────────────────────────────────
set(KHE_SOURCES
    src/main.cpp
    src/Theme.cpp
    src/Database.cpp
    src/Utils.cpp
    src/Controls.cpp
    src/MainWindow.cpp
    src/Dashboard.cpp
    src/PathsPage.cpp
    src/NotesPage.cpp
    src/NoteViewerPage.cpp
    src/AddPathDialog.cpp
    src/NoteEditorDialog.cpp
    src/SettingsPage.cpp
    src/QuickLaunch.cpp
)

set(KHE_RESOURCES
    src/resources.rc
)

add_executable(KHE WIN32 ${KHE_SOURCES} ${KHE_RESOURCES})

target_include_directories(KHE PRIVATE src/)

target_compile_definitions(KHE PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0601   # Windows 7+
    WINVER=0x0601
    _WIN32_IE=0x0900
)

target_link_libraries(KHE PRIVATE
    sqlite3
    comctl32
    comdlg32
    shell32
    gdiplus
    uxtheme
    shlwapi
    version
    dwmapi
)

if(MSVC)
    target_compile_options(KHE PRIVATE /W3 /MP /utf-8)
    # Embed the application manifest
    target_link_options(KHE PRIVATE
        "/MANIFEST:EMBED"
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/src/app.manifest"
    )
    set_target_properties(KHE PROPERTIES
        VS_DPI_AWARE "PerMonitor"
    )
endif()
