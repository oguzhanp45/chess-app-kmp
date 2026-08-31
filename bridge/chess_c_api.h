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
//   Kopru (JNI / cinterop) dizeyi zaten hemen kendi diline kopyalar.
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


// ---------------- ARAMA ----------------
// Bu fonksiyonlar BLOKE EDER ve arka planda cagrilmalidir. Suresince
// tutamacin sahibidirler: ayni tutamaca baska bir cagri YAPILMAMALIDIR.
// Tek istisna chess_stop ve chess_info_* ailesi.
//
// maxDepth 0 verilirse 64 kullanilir (pratikte sinirsiz).

// Motorun oynayacagi hamle, UCI. Seviye ayarina ve acilis kitabina uyar.
const char* chess_best_move(chess_engine* engine, int timeMs, int maxDepth);

// En iyi n hamle, skorlari ve varyantlariyla:
//
// {"moves":[{"uci":"e2e4","scoreCp":34,"mateIn":0,"pv":["e2e4","e7e5"]}]}
//
// ANALIZ icindir: seviye ayari ve kitap YOK SAYILIR, arama her zaman tam
// gucte yapilir. Ayni derinlikte chess_best_move'dan 2-4 kat yavastir.
// Mat/pat durumunda {"moves":[]} doner.
const char* chess_best_moves_json(chess_engine* engine, int n, int timeMs, int maxDepth);

// Pozisyonun anlik degerlendirmesi: {"scoreCp":34,"mateIn":0}
// Mikrosaniyeler surer; eval bar her hamleden sonra cagirabilir.
// Bitmis pozisyonda anlamli sayi vermez -- once chess_snapshot_json'daki
// status'e bakilmali.
const char* chess_evaluate_json(chess_engine* engine);

// Acilis kitabinin bu pozisyonda onerdikleri, agirliga gore azalan:
//
// {"moves":[{"uci":"e2e4","weight":8000,"percent":42}]}
//
// Arama YAPMAZ. chess_set_use_book(0) bunu ETKILEMEZ -- o ayar motorun
// OYNARKEN kitabi kullanip kullanmadigini belirler, gezgin her zaman calisir.
const char* chess_book_moves_json(chess_engine* engine);

// Aramayi keser. BASKA BIR IS PARCACIGINDAN CAGRILABILIR -- motor tarafinda
// yalnizca atomik bir bayrak set ediyor.
void chess_stop(chess_engine* engine);

// ---------------- CANLI ARAMA BILGISI ----------------
// Bu fonksiyonlar ARAMA SURERKEN cagrilabilir. Motora dokunmuyorlar; C
// katmanindaki atomik alanlari okuyorlar. Alanlari motorun info geri
// cagirimi dolduruyor, her yeni derinlikte guncelleniyor.
//
// Her arama basinda sifirlanirlar.

int       chess_info_depth(chess_engine* engine);
int       chess_info_sel_depth(chess_engine* engine);
int       chess_info_score_cp(chess_engine* engine);   // mateIn != 0 iken anlamsiz
int       chess_info_mate_in(chess_engine* engine);    // 0 = zorunlu mat yok
long long chess_info_nodes(chess_engine* engine);
int       chess_info_time_ms(chess_engine* engine);

// ---------------- SON ARAMANIN SONUCU ----------------
// Arama bittikten SONRA okunur.

int chess_last_score(chess_engine* engine);
int chess_last_depth(chess_engine* engine);
int chess_last_skill_loss(chess_engine* engine);   // seviye yuzunden feda edilen cp

// ---------------- AYARLAR ----------------

// 0 (en zayif) - 20 (tam guc). chess_best_move'u etkiler, best_moves'u ASLA.
// Seviye derinlik sinirini belirler: 1 + (level * 9) / 20.
void chess_set_skill_level(chess_engine* engine, int level);
int  chess_get_skill_level(chess_engine* engine);

// 1..1024 araligina kirpilir.
void chess_set_hash_mb(chess_engine* engine, int mb);

// Motorun OYNARKEN kitabi kullanip kullanmadigi.
void chess_set_use_book(chess_engine* engine, int on);

// Kitap mobilde dosya degil varlik oldugu icin bellekten yuklenir.
int chess_load_book_from_memory(chess_engine* engine,
                                const unsigned char* bytes, size_t size);
int chess_is_book_loaded(chess_engine* engine);

#ifdef __cplusplus
}
#endif

#endif // CHESS_C_API_H
