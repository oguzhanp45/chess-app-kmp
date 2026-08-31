#pragma once
#include "Board.hpp"
#include "MoveGen.hpp"
#include "Polyglot.hpp"
#include <atomic>
#include <random>
#include <utility>
#include <chrono>
#include <functional>
#include <string>
#include <vector>

const int HASH_EXACT = 0;
const int HASH_ALPHA = 1;
const int HASH_BETA = 2;

// ============================================================
//  MAT SKORLARI VE PLY
// ============================================================
// Onceki surumde mat skoru sabit 900000'di; bu yuzden mat-in-1 ile
// mat-in-7 ayni puani aliyordu ve motor en hizli mati secemiyordu.
// Artik skor koke olan uzakligi (ply) icinde tasiyor:
//     mat skoru = MATE_VALUE - ply
// Boylece daha yakin mat her zaman daha yuksek puan alir.
const int MAX_PLY = 128;
// PV'de saklanan en fazla hamle sayisi. 64 yarim hamle her arayuz icin
// fazlasiyla yeterli ve tabloyu [128][128] yerine [128][64] tutuyor.
const int MAX_PV = 64;
// Transpozisyon tablosu icin ust sinir. UCI'da ilan ettigimiz deger ile
// ayni; cok buyuk bir istek geldiginde motor asiri bellek ayirmasin.
const int MAX_HASH_MB = 1024;
const int MATE_VALUE = 900000;      // 0. ply'da mat
const int MATE_THRESHOLD = 800000;  // |skor| bunun ustundeyse bu bir mat skorudur
const int INFINITE_SCORE = 1000000;

// TT girisi 40 -> 24 bayt.
// Alan sirasi hizalama bosluklarini en aza indirecek sekilde secildi.
// depth ve flags int8 icin fazlasiyla yeterli (derinlik <= 127, bayrak 0-2).
// Kok hamlelerinden birinin tam sonucu: skor, hamle ve beklenen hat.
// bestMoves (multi-PV) bunlari siralayip ust n tanesini doner.
struct RootLine {
    int score = 0;
    Move move;
    std::vector<Move> pv;   // ilk eleman move ile ayni
};

struct TTEntry {
    unsigned long long key = 0;
    int value = 0;
    Move bestMove;
    int8_t depth = -1;
    int8_t flags = -1;
};

class AI {
private:
    MoveGenerator moveGen;

    // 64 -> MAX_PLY. Onceki boyutta "go depth 64" ile kok
    // siralamasi killerMoves[64]'e, yani dizi disina erisiyordu.
    Move killerMoves[MAX_PLY][2];

    // ---------------- UCGEN PV TABLOSU ----------------
    // pvTable[ply] = o dugumden itibaren motorun bekledigi hat.
    // Bir hamle alpha'yi yukselttiginde "o hamle + cocugun hatti" yukari
    // kopyalanir. Arama KARARLARINA karismaz, yalnizca kayit tutar --
    // bu yuzden dugum sayilari degismemelidir.
    Move pvTable[MAX_PLY][MAX_PV];
    int  pvLength[MAX_PLY] = {};

    int historyMoves[2][8][8][8][8] = {};

    Move counterMoveTable[8][8][8][8] = {};

    // hamle siralama icin sabit calisma alanlari.
    // Skorlar artik dugum basina BIR KEZ hesaplaniyor; onceki surumde
    // karsilastirma fonksiyonunun icinde, yani O(n log n) kez.
    struct OrderEntry { int score; int index; };
    static const int MAX_MOVES = 256;
    OrderEntry orderScratch[MAX_MOVES];
    Move moveScratch[MAX_MOVES];

    int rootDepth = 1;   // uzatma tavani icin

    // TT boyutu artik calisma zamaninda ayarlanabiliyor.
    // Mobilde 40 MB'lik tek tahsis dusuk segment cihazlarda sorun cikarir;
    // "setoption name Hash value 16" ile kuculebiliyor.
    size_t ttEntries = 1u << 20;
    size_t ttMask = (1u << 20) - 1;
    std::vector<TTEntry> transpositionTable;

    // ply basina hamle tamponu. Arama sirasinda hic heap tahsisi
    // yapilmamasi icin bir kez ayrilip surekli yeniden kullaniliyor.
    std::vector<Move> moveBuf[MAX_PLY];

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    long long timeLimitMs = 0;
    bool stopSearch = false;
    std::atomic<bool> abortSearch{ false };
    PolyglotBook book;                        // kitap bir kez yuklenir

    std::function<void(const std::string&)> onInfo;
    void info(const std::string& msg) { if (onInfo) onInfo(msg); }

    // seviye sistemi
    int skillLevel = 20;
    std::mt19937 skillRng{ std::random_device{}() };
    std::vector<std::pair<int, Move>> rootScores;   // son tamamlanan yinelemenin kok skorlari

    std::vector<unsigned long long> searchHistory;

    // Tamamlanan son yinelemenin kok hatti; ilk eleman oynanan hamle.
    std::vector<Move> lastPv;

    // Genis kok modunda her kok hamlesi icin skor + hat.
    std::vector<RootLine> rootLines;
    std::vector<RootLine> lastRootLines;
public:
    AI() : transpositionTable(1u << 20) {
        for (int i = 0; i < MAX_PLY; i++) moveBuf[i].reserve(64);
    }

    void setHashSizeMB(int mb);

    // --- TEST/OLCUM ALTYAPISI ---
    long long nodes = 0;
    bool useBook = true;
    std::string bookPath = "book.bin";

    // --- MOTOR CEKIRDEGI ARTIK STDOUT'A YAZMIYOR ---
    // Tum "info ..." satirlari buraya gider; stdout'a basmak UCI
    // katmaninin (main.cpp) isi. Mobilde stdout okuyan kimse olmadigi
    // icin bu ayrim sart.
    void setInfoCallback(std::function<void(const std::string&)> cb) {
        onInfo = cb;
        book.onInfo = cb;
    }

    // --- DISARIDAN DURDURMA ---
    // Arama ayri bir is parcaciginda kosarken bu bayrak set edilir.
    void stop() { abortSearch.store(true, std::memory_order_relaxed); }
    bool isStopped() const { return abortSearch.load(std::memory_order_relaxed); }

    // --- son aramanin skoru (test ve raporlama icin) ---
    int lastScore = 0;
    int lastDepth = 0;
    int selDepth = 0;   // aramanin ulastigi en derin ply (qsearch dahil)
    int lastSkillLoss = 0;   // seviye yuzunden feda edilen santipiyon

    // Tamamlanan son yinelemenin beklenen hatti (bos olabilir).
    const std::vector<Move>& getPv() const { return lastPv; }

    // ---------------- GENIS KOK (MULTI-PV) ----------------
    // Normal aramada kokte alpha yukseldigi icin 2., 3., 4. hamleler
    // gercek skorlarini almaz; yalnizca en iyiden kotu damgasi yer.
    // Bu bayrak aciksa kokte alpha YUKSELTILMEZ ve aspiration penceresi
    // kullanilmaz; her kok hamlesi tam pencereyle aranip gercek skorunu
    // alir. Bedeli: kokte budama yok, arama 2-4 kat yavas.
    // Yalnizca ANALIZ icin; oynarken kapali kalmali.
    bool wideRoot = false;

    // Tamamlanan son yinelemenin kok hamleleri (siralanmamis).
    const std::vector<RootLine>& getRootLines() const { return lastRootLines; }

    // ============================================================
    //  SEVIYE SISTEMI
    // ============================================================
    // Tuketici bir satranc uygulamasinda en kritik ozellik budur.
    // Kullanicinin 900'de makul, 1500'de zorlayici bir rakip bulmasi
    // motorun ham gucunden cok daha onemli.
    //
    // Iki kaldirac kullaniyoruz:
    //   1) Derinlik siniri  - kaba ama guvenilir merdiven
    //   2) Hamle secim gurultus - en iyiye "yeterince yakin" hamleler
    //      arasindan agirlikli rastgele secim. Sabit sig arama yerine
    //      ara sira hata yapan bir rakip daha insani hissettiriyor.
    //
    // 20 = tam guc (hicbir sinir yok). 0 = en zayif.
    void setSkillLevel(int level);
    int  getSkillLevel() const { return skillLevel; }

    // Kitabi bellekten yukle (Android'de dosya yok, asset var)
    bool loadBookFromMemory(const unsigned char* bytes, size_t size) {
        return book.loadFromMemory(bytes, size);
    }

    // Kitabi diskten simdi yukle (yuklenmemisse). Aramada tembel yukleme
    // zaten var; bu, cagiranin sonucu ogrenmek istedigi hal.
    bool ensureBookLoaded() {
        if (book.isLoaded()) return true;
        return book.load(bookPath);
    }
    bool isBookLoaded() const { return book.isLoaded(); }
    void unloadBook() { book.unload(); }

    // Bu pozisyondaki kitap hamleleri, agirliklariyla (secim yapmaz).
    std::vector<BookEntry> getBookEntries(ChessBoard& board) {
        return book.getBookEntries(board, moveGen);
    }

    void clearTT();

    // TT doluluk orani, binde. UCI "info hashfull" alani icin.
    // Ilk 1000 girisi orneklemek yeterli: indeksleme hash tabanli
    // oldugu icin dagilim duzgun ve arama sirasinda maliyeti sifir.
    int hashFullPerMille() const;
    void clearHistory();

    bool isInCheck(ChessBoard& board);

    int see(ChessBoard& board, Move m);

    int evaluateBoard(ChessBoard& board);
    int getPieceValue(int pieceType);

    // ikinci parametre artik DERINLIK degil PLY.
    void orderMoves(std::vector<Move>& moves, ChessBoard& board, int ply, Move pvMove = Move(), Move prevMove = Move());

    // --- NEGAMAX ---
    // Skorlar sira kimdeyse ona gore. isMaximizingPlayer parametresi
    // ortadan kalkti; her dugum kendi acisindan maksimize ediyor.
    int evalSTM(ChessBoard& board);   // evaluateBoard'in sira-sahibine gore hali

    // Sessizlik aramali degerlendirme: once alim zincirleri cozulur,
    // sonra deger dondurulur. Sira sahibine gore.
    //
    // Neden duz evalSTM degil: statik degerlendirme asili taslari gormez.
    // Vezir alinmak uzereyken de "vezir tahtada" diye sayar. Sakin
    // pozisyonlarda ikisi birebir ayni sonucu verir; fark yalnizca
    // taktik anlarda cikar, ki eval bar'a en cok o anda bakilir.
    int evaluateQuiet(ChessBoard& board);
    int search(ChessBoard& board, int depth, int ply, int alpha, int beta,
        bool allowNullMove = true, Move prevMove = Move(), Move excludedMove = Move());
    int qsearch(ChessBoard& board, int alpha, int beta, int ply, int qsDepth = 0);

    Move getBestMoveTimed(ChessBoard& board, int maxDepth, long long limitMs, long long maxLimitMs = 0, std::vector<unsigned long long> gameHistory = {});
};
