## Build ve Release Alma Rehberi — KHE_Public

> Bu rehber, KHE projesini sıfırdan derlemeyi, sorunları gidermeyi ve Release `.exe` çıkarmayı adım adım açıklar.

---

## İçindekiler

1. [Gereksinimler](#1-gereksinimler)
2. [Proje Yapısı](#2-proje-yapısı)
3. [Build Klasörünü Oluşturma (CMake Configure)](#3-build-klasörünü-oluşturma-cmake-configure)
4. [Derleme (MSBuild / CMake Build)](#4-derleme-msbuild--cmake-build)
5. [Release Çıktısının Yeri](#5-release-çıktısının-yeri)
6. [Olası Hatalar ve Çözümleri](#6-olası-hatalar-ve-çözümleri)
7. [Temiz Build (Clean Rebuild)](#7-temiz-build-clean-rebuild)

---

## 1. Gereksinimler

| Araç | Minimum Sürüm | Açıklama |
|------|-----------------|----------|
| **Visual Studio** | 2022 (v17) | "Desktop development with C++" workload yüklü olmalı |
| **CMake** | 3.20 | VS ile birlikte gelir; ayrıca [cmake.org](https://cmake.org) |
| **Git** | herhangi | Kaynak kodunu klonlamak için |
| **İnternet bağlantısı** | — | CMake, SQLite3'ü ilk derlemede otomatik indirir |
| **Windows** | 7 veya üstü | Hedef platform |

---

## 2. Proje Yapısı

```
KHE_Public/
├── CMakeLists.txt          ← Ana yapılandırma dosyası
├── src/                    ← Kaynak kodlar
├── build/                  ← Derleme çıktıları (git'e eklenmez)
└── Project_Documantion/   ← Belgeler
```

CMake, SQLite3'ü **FetchContent** ile `build/_deps/` klasörüne otomatik indirir.
Manuel bir SQLite kurulumu gerekmez.

---

## 3. Build Klasörünü Oluşturma (CMake Configure)

Bu adım, Visual Studio proje dosyalarını (`.sln`, `.vcxproj`) oluşturur.

### PowerShell ile

```powershell
# 1. Proje köküne git
Set-Location "C:\...\KHE_Public"

# 2. build klasörünü oluştur ve içine gir
New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location build

# 3. CMake configure (64-bit, Visual Studio 2022)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# 4. Derleme

derlemek için;

cmake --build build --config Release
```

# 4. Eğer Cmake Not Recognized alınırsa şunu gir;

$env:Path += ";C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"

bu geçici olarak pathe koyar ve her terminalde en az bir kere girmek gerekir çalışması için



**Ne yapar?**
- `cmake ..` → bir üst klasördeki `CMakeLists.txt` dosyasını okur.
- `-G "Visual Studio 17 2022"` → Visual Studio 2022 için `.sln` ve `.vcxproj` dosyaları üretir.
- `-A x64` → 64-bit mimari için yapılandırır.
- SQLite3, bu aşamada internetten indirilir ve `build/_deps/` altına yerleştirilir.

> **Not:** bu komut sadece **bir kez** çalıştırılması yeterli. Kaynak dosyalara ekleme yapılmadıkça tekrarlamaya gerek yoktur.

---

## 4. Derleme (MSBuild / CMake Build)

### Yöntem A — CMake üzerinden (tavsiye edilen)

```powershell
# build/ klasörünün içindeyken:
cmake --build . --config Release
```

**Ne yapar?**
- `--build .` → bulunulan klasördeki `.sln`'i derler.
- `--config Release` → optimize edilmiş Release modunu seçer (Debug yerine).
- Tüm `.cpp` dosyalarını çift çekirdek üzerinde paralel derler (`/MP` bayrağı sayesinde).

### Yöntem B — MSBuild ile doğrudan

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
    KHE.sln `
    /p:Configuration=Release `
    /p:Platform=x64 `
    /nologo /v:minimal
```

**Ne yapar?**
- `KHE.sln` → hangi solution'un derleneceğini belirtir.
- `/p:Configuration=Release` → Release konfigürasyonunu seçer.
- `/p:Platform=x64` → 64-bit platform hedefi.
- `/nologo` → MSBuild splash mesajını gizler.
- `/v:minimal` → sadece uyarı ve hataları gösterir (çok ayrıntılı çıktı yerine).

### Yöntem C — Visual Studio IDE ile

1. `build/KHE.sln` dosyasını Visual Studio ile aç.
2. Üst araç çubuğundan **Release** ve **x64** seçildiğinden emin ol.
3. **Derleme → Çözümü Derle** (`Ctrl+Shift+B`).

---

## 5. Release Çıktısının Yeri

Başarılı bir derlemeden sonra çalıştırılabilir dosya şu yola çıkar:

```
build\Release\KHE.exe
```

Uygulama çalışmak için ayrıca harici bir `.dll` gerektirmez; tüm bağımlılıklar statik olarak bağlanmıştır.

Veritabanı dosyası (`khe.db`) ilk çalıştırmada şu konumda otomatik oluşturulur:

```
%APPDATA%\KHE\khe.db
```

---

## 6. Olası Hatalar ve Çözümleri

---

### HATA 1 — `cmake` komutu tanınmıyor

```
'cmake' is not recognized as an internal or external command
```

**Neden olur?** CMake, sistem `PATH`'ine eklenmemiştir.

**Çözüm:**

1. Visual Studio Installer'ı aç → **Modify** → **Individual Components** sekmesi.
2. Arama kutusuna `cmake` yaz.
3. **"C++ CMake tools for Windows"** kutusunu işaretle → **Modify**.
4. Yükleme tamamlandıktan sonra yeni bir PowerShell penceresi aç ve tekrar dene.

Ya da CMake'i bağımsız kur ve aşağıdaki tam yolu kullan:
```powershell
& "C:\Program Files\CMake\bin\cmake.exe" .. -G "Visual Studio 17 2022" -A x64
```

---

### HATA 2 — `MSBuild` komutu tanınmıyor

```
'msbuild' is not recognized as an internal or external command
```

**Neden olur?** MSBuild, `PATH`'de değil.

**Çözüm:**

```powershell
# Visual Studio'nun kurulu olduğu yolu bul
Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Filter "MSBuild.exe" | Select-Object FullName
```

Bulunan tam yolu kullan:
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" KHE.sln /p:Configuration=Release /p:Platform=x64
```

> **Alternatif:** **Developer PowerShell for VS 2022**'yi başlat menüsünden aç — bu terminalde `cmake` ve `msbuild` otomatik olarak PATH'dedir.

---

### HATA 3 — SQLite indirilemiyor (FetchContent hatası)

```
CMake Error: Failed to download sqlite-amalgamation-***.zip
```

**Neden olur?** İnternet bağlantısı yok veya SQLite CDN'e erişilemiyor.

**Çözüm:**

1. Tarayıcıdan şu adresi aç ve zip'i indir:
   `https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip`
2. İndirilen zip'i aç, içindeki 4 dosyayı (`sqlite3.c`, `sqlite3.h`, `sqlite3ext.h`, `shell.c`) şu klasöre koy:
   ```
   build/_deps/sqlite3_src-src/
   ```
3. CMake'i tekrar çalıştır:
   ```powershell
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

---

### HATA 4 — `RICHEDIT50W` sınıfı bulunamıyor (çalışma zamanı)

```
CreateWindowExW failed for RICHEDIT50W
```

**Neden olur?** `Msftedit.dll` yüklenmiyor.

**Çözüm:** `main.cpp`'de `LoadLibraryW(L"Msftedit.dll")` çağrısının varlığını doğrula. Bu DLL Windows ile birlikte gelir; yüklenmemişse Windows güncellemelerini kontrol et.

---

### HATA 5 — CMake cache yolu uyuşmazlığı

```
MSB3191: Cannot create directory ... same name as a file
```

**Neden olur?** Proje farklı bir konuma taşındıktan sonra `build/CMakeCache.txt` hâlâ eski yolu gösteriyor.

**Çözüm (Temiz Build):** Aşağıdaki [Temiz Build](#7-temiz-build-clean-rebuild) bölümüne bak.

---

### HATA 6 — Linker hatası: Eksik sembol

```
error LNK2019: unresolved external symbol ...
```

**Neden olur?** Yeni eklenen bir `.cpp` dosyası `CMakeLists.txt`'e eklenmemiş.

**Çözüm:**

1. `CMakeLists.txt`'i aç.
2. `set(KHE_SOURCES ...)` bloğuna eksik dosyayı ekle:
   ```cmake
   set(KHE_SOURCES
       src/main.cpp
       src/YeniDosya.cpp   # <-- buraya ekle
       ...
   )
   ```
3. CMake'i yeniden yapılandır:
   ```powershell
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```
4. Ardından derle.

---

### HATA 7 — Unicode derleme hatası (Türkçe karakter)

```
error C2001: newline in constant
```

**Neden olur?** Kaynak dosya UTF-8 BOM olmadan kaydedilmiş, `/utf-8` bayrağına rağmen MSVC yanlış yorumluyor.

**Çözüm:**

1. Sorunlu `.cpp`/`.h` dosyasını VS Code'da aç.
2. Sağ alt köşedeki **UTF-8** yazısına tıkla → **"Save with Encoding"** → **UTF-8 with BOM** seç.
3. Tekrar derle.

---

## 7. Temiz Build (Clean Rebuild)

Proje taşındıysa, cache bozuksa veya anlaşılmaz hatalar alınıyorsa **build klasörünü silerek** sıfırdan başla:

```powershell
# Proje kökünde:
Set-Location "C:\...\KHE_Public"

# Eski build klasörünü tamamen sil
Remove-Item -Recurse -Force build

# Yeni build klasörü oluştur
New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location build

# Yeniden yapılandır (SQLite tekrar indirilir)
cmake .. -G "Visual Studio 17 2022" -A x64

# Derle
cmake --build . --config Release
```

**Tüm bu adımlar neden gerekli?**
- `CMakeCache.txt` içinde proje kök yolu sabit kodlanmış olarak saklanır.
- Taşıma sonrası bu yol geçersizleşir ve MSBuild dizin oluşturmakta başarısız olur.
- `build/` klasörü silinince cache temizlenir ve CMake doğru yollarla yeniden başlar.

---

*Son güncelleme: Mart 2026*



































































































