#ifndef CHESS_C_API_H
#define CHESS_C_API_H

#include <stddef.h>

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
// TASARIM KURALLARI (gerekcesi docs/kotlin-defteri.md'de):
//   1. Skaler donenler skaler kalir  -- bool yerine int (0/1).
//   2. Degisken uzunluklu ya da cok alanli olanlar TEK JSON dizesi doner.
//      Ozellikle durum sorgulari chess_snapshot_json'da birlesir:
//      hamle basina bes gecis yerine bir gecis.
//   3. Canli arama bilgisi hic dize olmaz; atomik int getter'lar (1.6b).
//
// BELLEK SAHIPLIGI
//   const char* donduren her fonksiyonun tutamac icinde KENDI tamponu
//   vardir. Donen isaretci, AYNI tutamac uzerinde AYNI fonksiyon tekrar
//   cagrilana kadar gecerlidir. Cagiran taraf serbest birakmaz;
//   chess_free_string diye bir sey yoktur.
//   Koprü (JNI / cinterop) dizeyi zaten hemen kendi diline kopyalar.
//
// IS PARCACIGI GUVENLIGI
//   EngineApi is parcacigi guvenli DEGILDIR ve bu yuzey onu
//   degistirmiyor. Bir tutamaca ayni anda tek bir is parcacigi
//   dokunmalidir. Arama fonksiyonlari (1.6b) bloke eder ve arka planda
//   cagrilmalidir; o sirada ayni tutamaca baska cagri YAPILMAMALIDIR.

#ifdef __cplusplus
extern "C" {
#endif

// ---------------- SURUM VE OZ TEST ----------------

// Motorun surum dizesi. Ornek: "cpp-chess-engine 1.0 (C++17)"
// Tutamac gerektirmez, sabit bir isaretci dondurur.
const char* chess_version(void);

// Kopru oz testi. Yeni bir motor ornegi kurar ve baslangic
// pozisyonundaki legal hamle sayisini dondurur.
//   20 = beklenen deger
//   -1 = motor kurulamadi
int chess_selftest(void);

// ---------------- TUTAMAC ----------------

// Opak tutamac. Icerigi cagiran tarafi ilgilendirmez.
typedef struct chess_engine chess_engine;

// Yeni bir motor ornegi. Basarisizlikta NULL.
chess_engine* chess_create(void);

// Ornegi yok eder. NULL guvenlidir.
void chess_destroy(chess_engine* engine);

// ---------------- OYUN KURULUMU VE DURUM ----------------

// Yeni oyun. fen NULL ya da bos ise baslangic pozisyonu.
// FEN reddedilirse 0 doner ve tahta baslangic pozisyonuna alinir.
int chess_new_game(chess_engine* engine, const char* fen);

// Pozisyonun tam durumu, tek JSON nesnesi:
//
// {"fen":"...","side":"w","inCheck":false,"status":"ongoing",
//  "legal":["e2e4","e2e3",...],"history":["e4","e5"]}
//
//   side    "w" | "b"
//   status  ongoing | checkmate | stalemate |
//           draw-fifty | draw-repetition | draw-material
//           (checkmate'te KAYBEDEN taraf side'dir)
//   legal   UCI dizeleri ("e2e4", "e7e8q")
//   history SAN dizeleri ("e4", "Nf3", "O-O")
//
// Hata durumunda "{}" doner (tutamac NULL ise).
const char* chess_snapshot_json(chess_engine* engine);

// ---------------- HAMLE ----------------

// Legal degilse 0 doner ve tahta degismez.
int chess_make_move(chess_engine* engine, const char* uci);

// Geri alinacak hamle yoksa 0 doner.
int chess_undo(chess_engine* engine);

// Bir hamlenin SAN karsiligi. Hamle OYNANMADAN once sorulur.
// Legal degilse bos dize doner.
const char* chess_san_for(chess_engine* engine, const char* uci);

#ifdef __cplusplus
}
#endif

#endif // CHESS_C_API_H
