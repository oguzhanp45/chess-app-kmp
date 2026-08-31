#ifndef CHESS_C_API_H
#define CHESS_C_API_H

// ============================================================
//  chess_c_api -- motorun C yuzeyi
// ============================================================
// Bu dosya UYGULAMA deposuna aittir, motora degil. Satranc
// motorlarinda standart gomme arayuzu UCI'dir; C yuzeyine ihtiyac
// mobilde surec acip stdin/stdout borulayamamaktan doguyor.
//
// Neden C, C++ degil:
//   Android'de Kotlin motora JNI ile baglanir. iOS'ta Kotlin/Native
//   cinterop ile baglanir ve cinterop bir C BASLIGI ister, C++ sinifi
//   okuyamaz. Tek bir extern "C" yuzey iki tarafa da hizmet eder.
//
// Tasarim kurallari (gerekcesi docs/kotlin-defteri.md'de):
//   1. Skaler donenler skaler kalir.
//   2. Degisken uzunluklu / cok alanli olanlar tek JSON dizesi doner.
//   3. Canli arama bilgisi hic dize olmaz; atomik int getter'lar.
//
// BELLEK SAHIPLIGI: const char* donduren her fonksiyon, cagiran
// tarafin serbest birakmayacagi bir tampona isaret eder. Isaretci
// ayni tutamaca yapilacak bir sonraki cagriya kadar gecerlidir.
// Kopru tarafi (JNI / cinterop) dizeyi hemen kendi diline kopyalar.

#ifdef __cplusplus
extern "C" {
#endif

// Motorun surum dizesi. Ornek: "cpp-chess-engine 1.0 (C++17)"
// Tutamac gerektirmez, sabit bir isaretci dondurur.
const char* chess_version(void);

// Kopru oz testi. Yeni bir motor ornegi kurar ve baslangic
// pozisyonundaki legal hamle sayisini dondurur.
//   20  = beklenen deger, motor calisiyor
//   -1  = motor kurulamadi
// Baska bir sayi gelirse motor ayakta ama bir seyler yanlis.
int chess_selftest(void);

#ifdef __cplusplus
}
#endif

#endif // CHESS_C_API_H
