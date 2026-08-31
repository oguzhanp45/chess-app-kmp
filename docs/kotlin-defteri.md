# Kotlin defteri

Bu dosya, `chess-app-kmp` uzerinde calisirken ogrenilenlerin uzun halidir.
Sohbette kisa aciklamalar geciyor; buraya "neden boyle" yaziliyor.

Sira faz numarasina gore. Her baslik, o adimda gercekten karsilastigimiz
bir problemi anlatir.

---

## Faz 1.2 — KMP proje yapisi

### Uc modul ne ise yarar

Sihirbaz (Android Studio Quail 3 / 2026.1.3) su yapiyi uretti:

```
shared/       ortak Kotlin kodu  -> hem Android hem iOS derler
androidApp/   Android uygulamasi -> sadece Android
iosApp/       iOS uygulamasi     -> sadece iOS (Xcode projesi)
```

Bu yapi Mayis 2026'da degisti. Eskiden `composeApp/` diye tek bir modul
vardi ve hem ortak kutuphaneyi hem Android uygulamasini iceriyordu.
Ayrilmasinin sebebi AGP 9.0: artik uygulama giris noktasinin ortak koddan
ayri bir modulde olmasini zorunlu tutuyor.

### `commonMain` / `androidMain` / `iosMain`

`shared/src/` altindaki bu klasorler KMP'nin kalbi:

| Klasor | Kim derler | Ne yazilir |
|---|---|---|
| `commonMain` | herkes | platformdan bagimsiz her sey |
| `androidMain` | sadece Android | JNI cagrilari, Android API'leri |
| `iosMain` | sadece iOS | cinterop cagrilari, Apple API'leri |

`commonMain`'de `android.content.Context` yazamazsin -- derleyici izin
vermez. Bu bir kisitlama degil, isin garantisi: mimari kural 5'i
("her katman yalnizca bir altindakini tanir") derleyici zorluyor.

Compose Multiplatform'u en bastan acmamizin asil sebebi buydu. Android'e
ozel baslayip sonra ortaklastirmak dosya tasima isi degil; asil zahmet
ekran koduna sizmis platform API'lerini ayiklamak olurdu.

### Uretilen `Res` sinifi ve `rootProject.name` tuzagi

`commonMain/composeResources/` altindaki dosyalar icin Compose otomatik
bir `Res` sinifi uretiyor ve bu sinifin **paket adini `rootProject.name`'den
turetiyor.** Sihirbaz projeyi `MyChessappkmp` diye uretmisti, `App.kt`
soyle import ediyordu:

```kotlin
import mychessappkmp.shared.generated.resources.Res
```

`rootProject.name`'i `chess-app-kmp` yapinca uretilen paket degisti ve
import'lar bosa dustu. Cozum adi geri almak degil, uretilen paketi
sabitlemek:

```kotlin
// shared/build.gradle.kts
compose.resources {
    packageOfResClass = "com.oguzhanp.chess.resources"
}
```

Artik proje adi ne olursa olsun kirilmiyor.

---

## Faz 1.4 — Motoru Android icin derletmek

### Problem

Kotlin C++ ile dogrudan konusamaz. Arada iki katman var:

```
Kotlin  ->  JNI  ->  C++
```

1.4'te bu zincirin sadece **C++ ucunu** kurduk: motor Android icin
derlensin ve APK'nin icine girsin. Kotlin'den cagirma yolu 1.5-1.6.

### `.so` nedir

`.so` = "shared object", Android'in calistirabildigi makine kodu paketi.
Kotlin bir `.so` yukleyip icindeki fonksiyonlari cagirabilir; ham `.cpp`
dosyalarini cagiramaz. Bizim uretigimiz: `libchessjni.so`.

Iki mimari icin ayri ayri uretiliyor, cunku makine kodu islemciye ozel:

- `arm64-v8a` -> Xiaomi Pad 7 (gercek test cihazi)
- `x86_64`    -> Pixel 7 emulatoru (bilgisayarin islemcisi)

### Neden ayri bir `engine-android` modulu

`shared` modulu AGP 9'un yeni `com.android.kotlin.multiplatform.library`
eklentisini kullaniyor. Bu eklenti **`externalNativeBuild`, CMake, NDK ve
`jniLibs` desteklemiyor** -- hata degil, belgelenmis bir sinir. Google'in
onerdigi cozum: NDK isini klasik bir `com.android.library` modulunde tut,
KMP modulu onu bagimlilik olarak kullansin.

Yan faydasi: JNI'in tamami tek modulde kapali kaliyor. `shared` JNI
bilmiyor, sadece bir arayuz cagiriyor.

### Dort halka

**1. `engine/`** -- motorun C++ kaynaklari, `cpp-chess-engine` v1.0'dan
birebir kopya. Elle duzenlenmez; motorda degisiklik gerekirse orada
yapilir, yeni etiket atilir, buraya yeniden kopyalanir. Hangi surumden
geldigi `engine/VERSION.md`'de yaziyor.

**2. `engine-android/src/main/cpp/CMakeLists.txt`** -- CMake'e tarif:

```cmake
add_subdirectory("${CHESS_REPO_ROOT}/engine" engine_build)
add_library(chessjni SHARED chessjni.cpp)
target_link_libraries(chessjni PRIVATE chessengine)
```

Sirasiyla: motoru derle, `libchessjni.so`'yu olustur, motoru onun icine
gom. `add_subdirectory`'nin ikinci argumani (`engine_build`) zorunlu,
cunku kaynak klasor bu klasorun disinda -- CMake ciktiyi nereye
koyacagini bilmek istiyor.

Motorun kendi `CMakeLists.txt`'i `ANDROID` tanimliyken test ve UCI
hedeflerini otomatik atliyor, yani APK'ya 33 KB'lik test kodu girmiyor.

`set(CMAKE_CXX_STANDARD 17)` satirini kendi dosyamizda da tekrar
yazmamiz gerekti: motorun ayari alt dizin kapsaminda kaliyor, bizim
hedefimiz `EngineApi.hpp`'yi dahil ettigi icin ayni standarda ihtiyaci var.

**3. `engine-android/build.gradle.kts`** -- Gradle'a tarif: derlerken
CMake'i cagir, iki mimari icin. `abiFilters` burada tanimli.

**4. `shared/build.gradle.kts` satir 46** -- zinciri kapatan satir:

```kotlin
androidMain.dependencies {
    implementation(project(":engine-android"))
}
```

`androidApp` zaten `shared`'a bagli. Boylece `.so` kendiliginden APK'ya
akiyor.

### `chess_smoke_test()` neden var

Hicbir ise yaramiyor, bilerek:

```cpp
extern "C" int chess_smoke_test() {
    EngineApi engine;
    return static_cast<int>(engine.legalMoves().size());
}
```

Tek gorevi derleyicinin sikayet etmemesini saglamak. Derlenip
baglandiysa sunlarin hepsi calisiyor demektir: basliklar bulundu, 14
kaynak dosya NDK'nin clang'iyle C++17 olarak derlendi, `chessengine`
statik kutuphanesi olustu, `.so`'ya gomuldu.

1.5'te gercek C yuzeyi gelince bu fonksiyon silinecek.

### Dogrulama yontemi

`.so`'nun gercekten APK'ya girdigini gormek icin:

```bash
unzip -l androidApp/build/outputs/apk/debug/androidApp-debug.apk | grep '\.so'
```

Gradle'in "BUILD SUCCESSFUL" demesi yetmez -- derlenen bir seyin
paketlenmedigi durumlar oluyor. Ciktinin kendisine bakmak lazim.

### Bilinen eksik (Faz 8)

`abiFilters` sadece `engine-android`'de tanimli. Bu yuzden APK'da
`armeabi-v7a` ve `x86` dilimleri de var (Compose bagimliliklarindan
geliyor) ama o mimarilerde `libchessjni.so` yok. 32-bit bir cihaza
kurulursa motor yuklenemez. Release APK'da `androidApp`'e de ayni filtre
konacak.

---

## C sinirinin tasarimi (1.5'te uygulanacak)

`bestMoves`, `bookMoves`, `legalMoves`, `moveHistorySan` -- dordu de
`std::vector` donduruyor ve C sinirindan gecemez. Ucunu de degerlendirdik:

| | JSON | Duz tampon | Sayac + indeks |
|---|---|---|---|
| `legalMoves` | kolay | kolay | kolay |
| `bestMoves` (ic ice) | bedava | boyut tahmini gerek | ic ice imlec API'si |
| Kotlin kodu | tek ayristirici | Android/iOS ayri | iki tarafta dongu |
| Gizli durum | yok | yok | var (riskli) |

**Performans bu karari vermiyor.** `legalMoves` en fazla ~30 tane 4-5
karakterlik dize; JNI gecisi onlarca nanosaniye. Ucu de mikrosaniyeler
mertebesinde, Compose'un kare butcesi 16 milisaniye. Asil ayrim
`bestMoves`: `vector<ScoredMove>` ve her elemanin icinde `vector<string> pv`
var, yani ic ice iki kat degisken uzunluk.

### Secilen karma yaklasim

1. **Skalerler skaler kalir.** `chess_make_move` bir `int` donduruyor,
   JSON'a sarmak sacma.

2. **Degisken uzunluklu / cok alanli olanlar tek JSON dizesi doner**, ve
   durum sorgulari tek cagrida birlesir:

   ```c
   const char* chess_snapshot_json(chess_engine*);
   /* {"fen":...,"side":...,"inCheck":...,"status":...,
       "legal":[...],"history":[...]} */
   ```

   `getFen` + `sideToMove` + `inCheck` + `gameStatus` + `legalMoves` +
   `moveHistorySan` yerine tek gecis. Mimari kural 4'un istedigi tam
   olarak bu.

   Elle JSON yazmak zahmetli degil: sinirdan gecen her dize UCI, SAN, FEN
   ya da durum adi; hicbirinde `"` veya `\` yok.

3. **Canli arama bilgisi hic dize olmaz.** `setInfoCallback`'i sinirdan
   gecirmek en pahali fikirdi: Android'de arama is parcacigindan JVM'e
   donmek `AttachCurrentThread` istiyor, iOS'ta `staticCFunction` yakalama
   yapamiyor. Yerine C katmani `info` satirini `std::atomic` alanlara
   yaziyor, Kotlin dort tane `int` okuyor:

   ```c
   int chess_info_depth(chess_engine*);
   int chess_info_score_cp(chess_engine*);
   int chess_info_mate_in(chess_engine*);
   long long chess_info_nodes(chess_engine*);
   ```

   Atomik olduklari icin **arama surerken cagrilmalari guvenli** --
   `EngineApi`'ye dokunmuyorlar.

### Bellek sahipligi kurali

`const char*` donduren her fonksiyon, o tutamacin ic tamponuna isaret
eder ve **ayni tutamaca yapilacak bir sonraki cagriya kadar gecerlidir.**
`chess_free_string` yok, sizinti yok, iki dilde de omur yonetimi yok.
JNI tarafi pointer'i hemen `NewStringUTF` ile kopyaliyor; cinterop tarafi
`.toKString()` ile ayni seyi yapiyor.

---

## Calisma yontemine dair dersler

**Ic ice parantezli degisikliklerde satir numarasi tarif etme.**
`compose.resources` blogunu satir numarasiyla tarif ettik, fazladan bir
`}` olustu, 23 derleme hatasi ve iki tur kayip. Tek satirlik degisiklikler
icin satir numarasi iyi calisiyor; blok ekleyen degisikliklerde dosyanin
tamami gonderilmeli.

**Yeniden adlandirmanin yan etkilerini onceden dusun.**
`rootProject.name` degisikligi zararsiz gorunuyordu ama uretilen kaynak
sinifinin paketini kirdi.
