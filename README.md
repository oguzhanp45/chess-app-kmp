# chess-app-kmp

[![CI](https://github.com/oguzhanp45/chess-app-kmp/actions/workflows/ci.yml/badge.svg)](https://github.com/oguzhanp45/chess-app-kmp/actions/workflows/ci.yml)

Kotlin Multiplatform satranç uygulaması — Android ve iOS. Satranç mantığının
tamamı [cpp-chess-engine](https://github.com/oguzhanp45/cpp-chess-engine)
adlı C++17 motorundan geliyor; Kotlin tarafında tek satır satranç kuralı yok.

A Kotlin Multiplatform chess app for Android and iOS. All chess logic comes
from the C++17 engine [cpp-chess-engine](https://github.com/oguzhanp45/cpp-chess-engine);
there is not a single chess rule written in Kotlin.

---

## Türkçe

### Depo düzeni

| Klasör | İçerik |
|---|---|
| `engine/` | Motor kaynaklarının kopyası. **Elle düzenlenmez** — hangi sürümden geldiği `engine/VERSION.md`'de |
| `bridge/` | `chess_c_api` — motorun `extern "C"` yüzeyi ve masaüstü testi `capitest` |
| `engine-android/` | Android kütüphane modülü: CMake/NDK derlemesi ve JNI shim'i |
| `shared/` | Ortak Kotlin ve Compose Multiplatform kodu |
| `androidApp/` | Android uygulama giriş noktası |
| `iosApp/` | iOS uygulama giriş noktası (Xcode projesi) |

### Katman şeması

```
EngineApi (C++ sınıf)          motor deposunda, dondurulmuş
    ↓
chess_c_api.h  extern "C"      bridge/
    ↓                ↓
JNI shim         cinterop
(Android)        (iOS, Faz 1.7 sonrası)
    ↓                ↓
ChessEngine (commonMain, expect/actual)
    ↓
Compose ekranı
```

`chess_c_api` neden var: Android'de Kotlin motora JNI ile bağlanır, iOS'ta
Kotlin/Native cinterop ile. cinterop bir **C başlığı** ister, C++ sınıfı
okuyamaz. Tek bir `extern "C"` yüzey iki tarafa da hizmet eder.

### Çalıştırma

```bash
# Android
./gradlew :androidApp:installDebug
adb shell am start -n com.oguzhanp.chess/.MainActivity

# iOS — Xcode gerekir
open iosApp/iosApp.xcodeproj
```

### Test

```bash
# C yüzeyi (masaüstü, cihaz gerekmez)
cmake -B build-desktop -DCHESS_BUILD_TOOLS=OFF
cmake --build build-desktop --config Release
ctest --test-dir build-desktop -C Release --output-on-failure

# iOS hedefleri derleniyor mu (Mac gerekir)
./gradlew :shared:linkDebugFrameworkIosSimulatorArm64
```

`-DCHESS_BUILD_TOOLS=OFF` zorunlu: `engine/` klasörü motor deposunun yalnızca
`src/` kısmını içerir, `tests/` ve `uci/` kopyalanmadı.

### Gereksinimler

Android Studio Otter (2025.2.1) veya üstü · JDK 17 · Android SDK + NDK ·
CMake 3.16+ · minSdk 26

---

## English

### Layout

| Folder | Contents |
|---|---|
| `engine/` | A copy of the engine sources. **Never edited by hand** — see `engine/VERSION.md` for the tag it came from |
| `bridge/` | `chess_c_api` — the engine's `extern "C"` surface, plus the desktop test `capitest` |
| `engine-android/` | Android library module: the CMake/NDK build and the JNI shim |
| `shared/` | Shared Kotlin and Compose Multiplatform code |
| `androidApp/` | Android application entry point |
| `iosApp/` | iOS application entry point (Xcode project) |

### Why `chess_c_api` exists

On Android, Kotlin reaches the engine through JNI. On iOS, it reaches it
through Kotlin/Native cinterop — and cinterop reads a **C header**, not a C++
class. One `extern "C"` surface serves both.

The C surface is part of this repository, not of the engine: the standard
embedding interface for a chess engine is UCI, and the need for a C surface
comes from not being able to spawn a process and pipe stdin/stdout on mobile.
That is a requirement of this app, not a gap in the engine.

### Running

```bash
# Android
./gradlew :androidApp:installDebug
adb shell am start -n com.oguzhanp.chess/.MainActivity

# iOS — requires Xcode
open iosApp/iosApp.xcodeproj
```

### Tests

```bash
# The C surface (desktop, no device needed)
cmake -B build-desktop -DCHESS_BUILD_TOOLS=OFF
cmake --build build-desktop --config Release
ctest --test-dir build-desktop -C Release --output-on-failure

# Do the iOS targets still link? (requires a Mac)
./gradlew :shared:linkDebugFrameworkIosSimulatorArm64
```

`-DCHESS_BUILD_TOOLS=OFF` is required: `engine/` holds only the `src/` part of
the engine repository — `tests/` and `uci/` were not copied.

### Requirements

Android Studio Otter (2025.2.1) or newer · JDK 17 · Android SDK + NDK ·
CMake 3.16+ · minSdk 26

---

## Lisans / License

Bu deponun kendi kodu için lisans henüz belirlenmedi.

Kullanılan üçüncü taraf varlıklar ve kütüphaneler — motor (MIT),
Material Symbols ikonları (Apache 2.0) ve diğerleri — için
[`docs/lisanslar.md`](docs/lisanslar.md).

No license decided yet for this repository's own code. For third-party
assets and libraries see [`docs/lisanslar.md`](docs/lisanslar.md).
