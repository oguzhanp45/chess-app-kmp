#pragma once
#include "Board.hpp"
#include "MoveGen.hpp"
#include <string>
#include <vector>
#include <random>
#include <functional>

// ============================================================
//  KITAP ARTIK BIR NESNE, DOSYA BIR KEZ OKUNUYOR
// ============================================================
// Onceki surumde getBookMove her hamlede .bin dosyasini aciyor, diskte
// binary search yapiyor ve "Kitapta toplam N kayit" satirini basiyordu.
// Ayrica srand(time(NULL)) her cagrida yeniden tohumlaniyordu; ayni saniye
// icindeki iki hamle ayni "rastgele" secimi uretiyordu.
// Artik: dosya bir kez bellege aliniyor, arama bellekte yapiliyor,
// rastgelelik mt19937 ile bir kez tohumlaniyor.
// Kitaptaki tek bir kayit: hamle ve agirligi.
// Agirlik, kitabin kaynak oyunlarinda o hamlenin ne siklikta oynandigi.
struct BookEntry {
    Move move;
    unsigned int weight = 0;
};

class PolyglotBook {
public:
    // Motor cekirdegi dogrudan stdout'a yazmaz; mesajlar buraya gider.
    std::function<void(const std::string&)> onInfo;

    bool load(const std::string& path);   // basarisiz olursa bir daha denemez

    // Android'de kitap dosya sisteminde degil asset icinde olur;
    // kopru byte dizisini dogrudan buraya verir.
    bool loadFromMemory(const unsigned char* bytes, size_t size);
    bool isLoaded() const { return loaded; }
    void unload() { data.clear(); loaded = false; loadFailed = false; }

    // Kitapta karsilik yoksa null Move doner.
    Move getBookMove(ChessBoard& board, MoveGenerator& moveGen);

    // Bu pozisyon icin kitaptaki TUM legal hamleler, agirliga gore
    // azalan sirali. Acilis gezgini icin; secim yapmaz.
    std::vector<BookEntry> getBookEntries(ChessBoard& board, MoveGenerator& moveGen);

    // Sifirdan hesaplanan Polyglot anahtari - artimli zobristKey'in
    // dogrulama referansi olarak duruyor (perftverify ve "d" komutu).
    static unsigned long long computeHash(ChessBoard& board);

private:
    std::vector<unsigned char> data;
    bool loaded = false;
    bool loadFailed = false;
    std::mt19937 rng{ std::random_device{}() };

    void info(const std::string& msg) { if (onInfo) onInfo(msg); }

    static unsigned long long readKey(const unsigned char* p);
    static unsigned short readU16(const unsigned char* p);

    // Bu pozisyonun ham kitap kayitlari (16 bitlik hamle + agirlik),
    // dosyadaki sirayla. getBookMove ve getBookEntries ayni taramayi
    // kullansin diye ayrildi.
    bool rawEntries(ChessBoard& board,
        std::vector<unsigned short>& moves,
        std::vector<unsigned int>& weights,
        unsigned long long& totalWeight);

    // 16 bitlik Polyglot kodunu Move'a cevirir (legallik KONTROL ETMEZ).
    static Move decodeMove(unsigned short code, ChessBoard& board);
};