#pragma once
#include "Board.hpp"
#include "MoveGen.hpp"
#include "AI.hpp"
#include <string>
#include <vector>
#include <functional>

// ============================================================
//  GOMME CEPHESI
// ============================================================
// Bu sinif, motorun disariya bakan tek yuzu. Motoru kendi icine
// gomen bir uygulama SADECE buraya baglanir.
//
// Neden gerekli:
//   1) Dil sinirindan gecen her sey basit tipte olmali. Move
//      struct'ini baska bir dile gecirmek eziyettir; "e2e4"
//      gecirmek bedavadir. Ceviriyi tek yerde yapiyoruz.
//   2) UCI metin protokolu bir ARAYUZ degil, bir PROTOKOL. Gomulu
//      kullanimda metin ayristirmak sacma olur.
//   3) Motorun ic yapisi degistiginde (Move'un boyutu, TT duzeni...)
//      cagiran kod etkilenmez. Sinir burasi.
//
// UCI katmani (uci/main.cpp) bu sinifi kullanmaz; ikisi kardestir.
// Boylece motoru hem satranc arayuzleriyle test edebiliyor hem de
// baska bir uygulamaya gomebiliyoruz.
// bestMoves'un dondurdugu tek bir aday.
//   scoreCp  santipiyon, SIRA SAHIBINE gore (+ = sirasi gelen lehine)
//   mateIn   0 = mat yok, +n = n hamlede mat ediyor, -n = n hamlede mat oluyor
//            mateIn != 0 iken scoreCp anlamsizdir.
//   pv       beklenen hat, UCI olarak; ilk eleman uci ile aynidir
// Bir pozisyonun degerlendirmesi. ScoredMove ile AYNI iki alan, ayni
// kural: arayuz motor skorlarini tek bir bicimde okur.
// Acilis kitabindaki bir hamle.
//   weight   kitabin kaynak oyunlarinda oynanma agirligi (ham deger)
//   percent  bu pozisyondaki toplam agirligin yuzdesi, 0-100
struct BookMove {
    std::string uci;
    int weight = 0;
    int percent = 0;
};

struct Evaluation {
    int scoreCp = 0;   // santipiyon, SIRA SAHIBINE gore
    int mateIn = 0;    // 0 = zorunlu mat yok, +n = mat ediyor, -n = mat oluyor
};

struct ScoredMove {
    std::string uci;
    int scoreCp = 0;
    int mateIn = 0;
    std::vector<std::string> pv;
};

// ============================================================
//  IS PARCACIGI GUVENLIGI
// ============================================================
// Bu sinif is parcacigi guvenli DEGILDIR. Bir EngineApi ornegine
// ayni anda yalnizca tek bir is parcacigi dokunmalidir.
//
// Tek istisna stop(): arama surerken baska bir is parcacigindan
// cagrilabilir, cunku yalnizca bir atomik bayrak set eder.
//
// Pratikte bu su demek: bestMove(), bestMoves() ve evaluate()
// bloke eder ve arka planda kosturulur. O sirada arayuz
// makeMove(), undo(), legalMoves() gibi bir cagri YAPMAMALIDIR --
// hata almaz, sessizce bozuk sonuc alir. Arama sirasinda gelen
// sorular cagiran tarafta onbellekten cevaplanmalidir.
class EngineApi {
public:
    EngineApi();

    // ---------------- OYUN KURULUMU ----------------
    // fen bos birakilirsa baslangic pozisyonu kullanilir.
    bool newGame(const std::string& fen = "");

    // ---------------- DURUM SORGULARI ----------------
    std::string getFen();
    std::string sideToMove();                  // "w" veya "b"
    bool inCheck();

    // Su an oynanabilecek tum hamleler, UCI formatinda ("e2e4", "e7e8q").
    std::vector<std::string> legalMoves();

    // "ongoing" | "checkmate" | "stalemate" |
    // "draw-fifty" | "draw-repetition" | "draw-material"
    // checkmate durumunda kaybeden taraf sideToMove()'dur.
    std::string gameStatus();

    // Oynanan hamleler, SAN formatinda ("e4", "Nf3", "O-O", "Qxd5+").
    std::vector<std::string> moveHistorySan();

    // ---------------- HAMLE ----------------
    bool makeMove(const std::string& uci);     // legal degilse false, tahta degismez
    bool undo();                               // geri alinacak hamle yoksa false

    // Bir hamlenin SAN karsiligi (hamle OYNANMADAN once sorulur).
    std::string sanFor(const std::string& uci);

    // ---------------- ARAMA ----------------
    // Bloke eder. Cagiran taraf bunu arka plan is parcaciginda cagirmali.
    std::string bestMove(int timeMs, int maxDepth = 64);

    // En iyi n hamle, skorlariyla ve beklenen hatlariyla (multi-PV).
    // Bloke eder; arka plan is parcaciginda cagrilmali.
    //
    // ANALIZ icindir: seviye ayari ve acilis kitabi YOK SAYILIR, arama
    // her zaman tam gucte yapilir. Sebep: egitmen "en iyi hamle suydu"
    // derken nesnel dogruyu gostermeli; zayiflatilmis bir motor kotu
    // bir hamleyi iyi gosterirdi.
    //
    // Kokte budama yapilmadigi icin ayni derinlikte bestMove'dan 2-4 kat
    // yavastir. Skora gore azalan sirada doner; legal hamle sayisi n'den
    // azsa daha az eleman doner, mat/pat durumunda bos doner.
    std::vector<ScoredMove> bestMoves(int n, int timeMs, int maxDepth = 64);

    // Pozisyonun anlik degerlendirmesi. Mikrosaniyeler surer; eval bar
    // her hamleden sonra bunu cagirabilir.
    //
    // Tam arama YAPMAZ, ama saf statik de degildir: once alim zincirleri
    // cozulur (sessizlik aramasi), sonra deger dondurulur. Sakin bir
    // pozisyonda ikisi ayni sonucu verir; asili tas varken saf statik
    // yaniltir. Ornek: siyah vezir alinmak uzereyken saf statik +129,
    // gercek +822 der.
    //
    // Bittigi belli pozisyonlarda (mat/pat) once gameStatus()'e bakilmali;
    // orada bu fonksiyon anlamli bir sayi vermez.
    Evaluation evaluate();

    // Bu pozisyonda acilis kitabinin onerdigi hamleler, agirliga gore
    // azalan sirali. Acilis gezgini icin; arama YAPMAZ, kitap yoksa
    // veya pozisyon kitapta bitmisse bos doner.
    //
    // setUseBook(false) bunu ETKILEMEZ: o ayar motorun OYNARKEN kitabi
    // kullanip kullanmadigini belirler, gezgin her durumda calismali.
    std::vector<BookMove> bookMoves();

    void stop();                               // baska bir is parcacigindan cagrilabilir
    int  lastScore() const { return ai.lastScore; }
    int  lastDepth() const { return ai.lastDepth; }
    int  lastSelDepth() const { return ai.selDepth; }   // ulasilan en derin ply
    long long lastNodes() const { return ai.nodes; }
    int  lastSkillLoss() const { return ai.lastSkillLoss; }   // seviye yuzunden feda edilen cp

    // ---------------- AYARLAR ----------------
    void setSkillLevel(int level);             // 0 (en zayif) - 20 (tam guc)
    int  getSkillLevel() const;
    void setHashSizeMB(int mb);
    void setUseBook(bool on);
    // Kitabi diskten yukler. Once bunu cagirmak zorunlu degil - arama
    // gerektiginde kendisi yukler - ama donen deger dosyanin gercekten
    // okunup okunmadigini soyler.
    bool loadBookFromFile(const std::string& path);
    bool isBookLoaded() const;
    bool loadBookFromMemory(const unsigned char* bytes, size_t size);

    // Arama sirasindaki "info depth ..." satirlari buraya akar.
    void setInfoCallback(std::function<void(const std::string&)> cb);

private:
    ChessBoard board;
    MoveGenerator moveGen;
    AI ai;

    std::vector<Move> playedMoves;                 // SAN gecmisi icin
    std::vector<unsigned long long> gameHistory;   // tekrar tespiti icin
    std::string startFen;

    bool findLegal(const std::string& uci, Move& out);
};
