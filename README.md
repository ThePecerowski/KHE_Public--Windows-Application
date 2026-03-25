# KHE — Kisisel Hizli Erisim Sistemi

Windows icin C++17 / Win32 API / SQLite tabanli kisisel hizli erisim uygulamasi.

## Ozellikler
- Dosya yolu kaydetme (Explorer'da ac / programi calistir)
- Rich-Text notlar (bold/italic/underline + resim/video/ses ekleri)
- Aydinlik ve Karanlik tema (anlik gecis)
- SQLite veritabani — sunucu gerekmez (%APPDATA%\KHE\khe_data.db)
- Breadcrumb gezgini

## Derleme

### Gereksinimler
- CMake >= 3.20
- MSVC (Visual Studio 2019/2022) veya MinGW-w64
- Internet (ilk derlemede SQLite otomatik indirilir)

### Adimlar
```powershell
cd KHE_Public
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Cikti: build\Release\KHE.exe

## Proje Yapisi
```
KHE_Public/
+-- CMakeLists.txt
+-- src/
    +-- main.cpp              (giris noktasi)
    +-- Models.h              (PathEntry, Note, NoteMedia)
    +-- Database.h/.cpp       (SQLite CRUD)
    +-- Theme.h/.cpp          (renk paleti)
    +-- Utils.h/.cpp          (yardimci fonksiyonlar)
    +-- AppContext.h          (global durum)
    +-- Controls.h/.cpp       (GDI cizim)
    +-- MainWindow.h/.cpp     (ana pencere + Sidebar + TopBar)
    +-- Dashboard.h/.cpp      (ana sayfa)
    +-- PathsPage.h/.cpp      (yol yonetimi)
    +-- NotesPage.h/.cpp      (not kartlari)
    +-- NoteViewerPage.h/.cpp (not goruntuleyici)
    +-- AddPathDialog.h/.cpp  (yol ekleme/duzenleme)
    +-- NoteEditorDialog.h/.cpp (rich-text editor)
    +-- resource.h / resources.rc / app.manifest
```

## Kullanim
| Eylem | Nasil |
|---|---|
| Yol ekle | Sidebar -> Dosya Yollari -> + Yeni Dosya Yolu Ekle |
| Dosyayi ac | Sag tikla -> Dosyayi/Programi Ac |
| Not ekle | Sidebar -> Notlar -> + Yeni Not Ekle |
| Notu goruntule | Kart uzerine tikla |
| Tema degistir | Sidebar alt kismi -> buton |
