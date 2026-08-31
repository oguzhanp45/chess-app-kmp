#pragma once
#include "Board.hpp"
#include <vector>

// ============================================================
//  HAMLE URETIMI ARTIK HEAP TAHSISI YAPMIYOR
// ============================================================
// Onceki surumde her tas icin ayri bir std::vector olusturuluyor,
// doldurulup ana listeye kopyalaniyor ve yok ediliyordu: dugum basina
// 16'ya varan heap tahsisi + kopya. Ustune getLegalMoves listeyi DEGERLE
// donduruyordu, yani bir tahsis daha. Artik tum uretimler cagiranin
// verdigi tampona ekleniyor; arama sirasinda tahsis yapilmiyor.
class MoveGenerator {
private:
    void getSliderMoves(int row, int col, const int* rowOffsets, const int* colOffsets,
        int dirCount, ChessBoard& board, std::vector<Move>& out);
    std::vector<Move> pseudoScratch;   // sozde-legal hamleler icin tek tampon

public:
    MoveGenerator() { pseudoScratch.reserve(256); }

    bool isWithinBoard(int row, int col);

    void getKnightMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);
    void getRookMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);
    void getBishopMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);
    void getQueenMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);
    void getPawnMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);
    void getKingMoves(int row, int col, ChessBoard& board, std::vector<Move>& out);

    void generatePseudoMoves(ChessBoard& board, std::vector<Move>& out);

    // Tampona yazan hizli surumler (arama bunlari kullanir)
    void getLegalMoves(ChessBoard& board, std::vector<Move>& out);
    void getLegalCaptures(ChessBoard& board, std::vector<Move>& out);

    // Kolaylik sarmalayicilari (test ve UCI katmani icin; sicak yolda degil)
    std::vector<Move> getLegalMoves(ChessBoard& board);
    std::vector<Move> getLegalCaptures(ChessBoard& board);
};