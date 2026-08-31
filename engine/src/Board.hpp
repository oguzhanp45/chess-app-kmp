#pragma once
#include "Types.hpp"
#include <vector>
#include <string>

// Baslangic pozisyonu. Onceden Tests.cpp icindeydi; motor cekirdegi
// (EngineApi) test dosyasina bagimli kaliyordu ve testler ayri bir
// hedefe alinamiyordu.
// inline constexpr (C++17) sayesinde ayri bir .cpp tanimina gerek yok - baslik kac yerden dahil edilirse edilsin tek nesne olusur.
inline constexpr const char* START_FEN =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


class ChessBoard {
private:
    int board[8][8] = {};

public:
    int currentPlayer = 1;
    int halfMoveClock = 0;
    int fullMoveNumber = 1;          // FEN cikti/girisi icin eklendi

    // --- INCREMENTAL HASHING ---
    unsigned long long zobristKey = 0;
    unsigned long long pieceHash = 0;

    bool whiteCastleK = true;
    bool whiteCastleQ = true;
    bool blackCastleK = true;
    bool blackCastleQ = true;

    int enPassantRow = -1;
    int enPassantCol = -1;

    // --- SAH KARESI ARTIMLI TAKIP ---
    // Onceki surumde sah karesi her ihtiyac duyuldugunda 64 kare taranarak
    // bulunuyordu. getLegalMoves bunu HER sozde-hamle icin yapiyordu:
    // dugum basina ~35 hamle x 64 kare = 2.240 gereksiz okuma.
    int whiteKingRow = 7, whiteKingCol = 4;
    int blackKingRow = 0, blackKingCol = 4;

    std::vector<MoveRecord> history;

    void undoMove();
    void initializeBoard();
    void printBoard();
    bool isWhite(int x, int y);
    int getPiece(int row, int col);

    void generateHash();

    // --- TEST ALTYAPISI ---
    bool setFen(const std::string& fen);   // basarisizsa false doner ve tahtayi bozmaz
    std::string getFen();
    unsigned long long debugRecomputePieceHash();  // artimli hash dogrulamasi icin

    bool isSquareAttacked(int row, int col, int attackerColor);
    bool isKingAttacked(int color);
    void refreshKingSquares();               // (setFen / initializeBoard sonrasi)
    bool debugKingSquaresValid();            // (perftverify icin)
    void makeMove(Move move);

    // --- BERABERLIK KURALLARI ---
    int  repetitionCount();          // mevcut pozisyon kac kez tekrarlandi (kendisi dahil)
    bool isInsufficientMaterial();   // K-K, K+A-K, K+F-K, ayni renk K+F-K+F
    bool isDraw();
};