#pragma once
#include <cstdint>

// --- TAS DEGERLERI (Polyglot Standartlari) ---
const int EMPTY = 0;

const int W_PAWN = 1;
const int W_KNIGHT = 2;
const int W_BISHOP = 3;
const int W_ROOK = 4;
const int W_QUEEN = 5;
const int W_KING = 6;

const int B_PAWN = -1;
const int B_KNIGHT = -2;
const int B_BISHOP = -3;
const int B_ROOK = -4;
const int B_QUEEN = -5;
const int B_KING = -6;

// ============================================================
//  MOVE 20 BAYTTAN 5 BAYTA INDI
// ============================================================
// Alanlar int (4 bayt) yerine int8_t. Degerler zaten -1..7 ve terfi
// icin -5..5 araliginda; int'in tamami bosa gidiyordu.
// Etkisi: hamle listeleri, transpozisyon tablosu, killer/counter
// tablolari ve MoveRecord gecmisi dortte birine iniyor - yani cok
// daha fazlasi onbellekte kaliyor.
//
// Kurucu eklendi cunku int8_t alanlara int deger veren suslu parantezli
// baslatma (push_back({r, c, tr, tc})) daraltma hatasi verirdi.
struct Move {
    int8_t startRow = -1;
    int8_t startCol = -1;
    int8_t targetRow = -1;
    int8_t targetCol = -1;
    int8_t promotionPiece = 0;

    Move() = default;
    Move(int sr, int sc, int tr, int tc, int promo = 0)
        : startRow((int8_t)sr), startCol((int8_t)sc),
        targetRow((int8_t)tr), targetCol((int8_t)tc),
        promotionPiece((int8_t)promo) {
    }

    bool isNull() const { return startRow == -1; }

    bool operator==(const Move& other) const {
        return startRow == other.startRow &&
            startCol == other.startCol &&
            targetRow == other.targetRow &&
            targetCol == other.targetCol &&
            promotionPiece == other.promotionPiece;
    }
};

struct MoveRecord {
    Move move;
    int capturedPiece = 0;

    bool whiteCastleK = false;
    bool whiteCastleQ = false;
    bool blackCastleK = false;
    bool blackCastleQ = false;
    int enPassantRow = -1;
    int enPassantCol = -1;
    int halfMoveClock = 0;

    // --- : O(1) HIZINDA GERI ALMA ICIN HASH HAFIZASI ---
    unsigned long long zobristKey = 0;
    unsigned long long pieceHash = 0;
};