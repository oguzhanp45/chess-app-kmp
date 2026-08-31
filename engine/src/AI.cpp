#include "AI.hpp"
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "Notation.hpp"
#include <sstream>
#include <chrono>

//main.cpp icindeki ceviri fonksiyonunu AI.cpp'ye tanitiyoruz

const int mg_value[7] = { 0,  82, 337, 365, 477, 1025, 10000 };
const int eg_value[7] = { 0,  94, 281, 297, 512,  936, 10000 };

const int mg_pawnTable[8][8] = {
    {  0,   0,   0,   0,   0,   0,   0,   0},
    { 98, 134,  61,  95,  68, 126,  34, -11},
    { -6,   7,  26,  31,  65,  56,  25, -20},
    {-14,  13,   6,  21,  23,  12,  17, -23},
    {-27,  -2,  -5,  12,  17,   6,  10, -25},
    {-26,  -4,  -4, -10,   3,   3,  33, -12},
    {-35,  -1, -20, -23, -15,  24,  38, -22},
    {  0,   0,   0,   0,   0,   0,   0,   0}
};

const int mg_knightTable[8][8] = {
    {-167, -89, -34, -49,  61, -97, -15, -107},
    { -73, -41,  72,  36,  23,  62,   7,  -17},
    { -47,  60,  37,  65,  84, 129,  73,   44},
    {  -9,  17,  19,  53,  37,  69,  18,   22},
    { -13,   4,  16,  13,  28,  19,  21,   -8},
    { -23,  -9,  12,  10,  19,  17,  28,   -9},
    { -41, -21, -13, -27, -21, -25, -38,  -29},
    { -98, -89, -53, -96, -76,-107, -37, -112}
};

const int mg_bishopTable[8][8] = {
    {-29,   4, -82, -37, -25, -42,   7,  -8},
    {-26,  16, -18, -13,  30,  59,  18, -47},
    {-16,  37,  43,  40,  35,  50,  37,  -2},
    { -4,   5,  19,  50,  37,  37,   7,  -2},
    { -6,  13,  13,  26,  34,  12,  10,   4},
    {  0,  15,  15,  15,  14,  27,  18,  10},
    {  4,  15,  16,   0,   7,  21,  33,   1},
    {-33,  -3, -14, -21, -13, -12, -39, -21}
};

const int mg_rookTable[8][8] = {
    { 32,  42,  32,  51,  63,   9,  31,  43},
    { 27,  32,  58,  62,  80,  67,  26,  44},
    { -5,  19,  26,  36,  17,  45,  61,  16},
    {-24, -11,   7,  26,  24,  35,  -8, -20},
    {-36, -26, -12,  -1,   9,  -7,   6, -23},
    {-45, -25, -16, -17,   3,   0,  -5, -33},
    {-44, -16, -20,  -9,  -1,  11,  -6, -71},
    {-19, -13,   1,  17,  16,   7, -37, -26}
};

const int mg_queenTable[8][8] = {
    {-28,   0,  29,  12,  59,  44,  43,  45},
    {-24, -39,  -5,   1, -16,  57,  28,  54},
    {-13, -17,   7,   8,  29,  56,  47,  57},
    {-27, -27, -16, -16,  -1,  17,  -2,   1},
    { -9, -26,  -9, -10,  -2,  -4,   3,  -3},
    {-14,   2, -11,  -2,  -5,   2,  14,   5},
    {-35,  -8,  11,   2,   8,  15,  -3,   1},
    { -1, -18,  -9,  10, -15, -25, -31, -50}
};

const int mg_kingTable[8][8] = {
    {-65,  23,  16, -15, -56, -34,   2,  13},
    { 29,  -1, -20,  -7,  -8,  -4, -38, -29},
    { -9,  24,   2, -16, -20,   6,  22, -22},
    {-17, -20, -12, -27, -30, -25, -14, -36},
    {-49,  -1, -27, -39, -46, -44, -33, -51},
    {-14, -14, -22, -46, -44, -30, -15, -27},
    {  1,   7,  -8, -64, -43, -16,   9,   8},
    {-15,  36,  12, -54,   8, -28,  24,  14}
};

const int eg_pawnTable[8][8] = {
    {  0,   0,   0,   0,   0,   0,   0,   0},
    {178, 173, 158, 134, 147, 132, 165, 187},
    { 94, 100,  85,  67,  56,  53,  82,  84},
    { 32,  24,  13,   5,  -2,   4,  17,  17},
    { 13,   9,  -3,  -7,  -7,  -8,   3,  -1},
    {  4,   7,  -6,   1,   0,  -5,  -1,  -8},
    { 13,   8,   8,  10,  13,   0,   2,  -7},
    {  0,   0,   0,   0,   0,   0,   0,   0}
};

const int eg_knightTable[8][8] = {
    {-58, -38, -13, -28, -31, -27, -63, -99},
    {-25,  -8, -25,  -2,  -9, -25, -24, -52},
    {-24, -20,  10,   9,  -1,  -9, -19, -41},
    {-17,   3,  22,  22,  22,  11,   8, -18},
    {-18,  -6,  16,  25,  16,  17,   4, -18},
    {-23,  -3,  -1,  15,  10,  -3, -20, -22},
    {-42, -20, -10,  -5,  -2, -20, -23, -44},
    {-29, -51, -23, -38, -29, -27, -43, -74}
};

const int eg_bishopTable[8][8] = {
    {-14, -21, -11,  -8,  -7,  -9, -17, -24},
    { -8,  -4,   7, -12,  -3, -13,  -4, -14},
    {  2,  -8,   0,  -1,  -2,   6,   0,   4},
    { -3,   9,  12,   9,  14,  10,   3,   2},
    { -6,   3,  13,  19,   7,  10,  -3,  -9},
    {-12,  -3,   8,  10,  13,   3,  -7, -15},
    {-14, -18,  -7,  -1,   4,  -9, -15, -27},
    {-23,  -9, -23,  -5,  -9, -16,  -5, -17}
};

const int eg_rookTable[8][8] = {
    { 13,  10,  18,  15,  12,  12,   8,   5},
    { 11,  13,  13,  11, -3,   3,   8,   3},
    {  7,   7,   7,   5,   4,  -3,  -5,  -3},
    {  4,   3,  13,   1,   2,   1,  -1,   2},
    {  3,   5,   8,   4,  -5,  -6,  -8, -11},
    { -4,   0,  -5,  -1,  -7, -12,  -8, -16},
    { -6,  -6,   0,   2,  -9,  -9, -11,  -3},
    { -9,   2,   3,  -1,  -5, -13,   4, -20}
};

const int eg_queenTable[8][8] = {
    { -9,  22,  22,  27,  27,  19,  10,  20},
    {-17,  20,  32,  41,  58,  25,  30,   0},
    {-20,   6,   9,  49,  47,  35,  19,   9},
    {  3,  22,  24,  45,  57,  40,  57,  36},
    {-18,  28,  19,  47,  31,  34,  12,  11},
    {-16, -27,  15,   6,   9,  17,  10,   5},
    {-22, -23, -30, -16, -16, -23, -36, -32},
    {-33, -28, -22, -43,  -5, -32, -20, -41}
};

const int eg_kingTable[8][8] = {
    {-74, -35, -18, -18, -11,  15,   4, -17},
    {-12,  17,  14,  17,  17,  38,  23,  11},
    { 10,  17,  23,  15,  20,  45,  44,  13},
    { -8,  22,  24,  27,  26,  33,  26,   3},
    {-18,  -4,  21,  24,  27,  23,   9, -11},
    {-19,  -3,  11,  21,  23,  16,   7,  -9},
    {-27, -11,   4,  13,  14,   4,  -5, -17},
    {-53, -34, -21, -11, -28, -14, -24, -43}
};

// ============================================================
//  TT'DE MAT SKORU DONUSUMU
// ============================================================
// Ayni pozisyon farkli ply'larda tabloya girebilir. Mat skoru koke olan
// uzakligi icerdigi icin tabloya YAZARKEN dugume gore, OKURKEN koke gore
// normalize edilmelidir. Aksi halde derin bir daldan gelen "mat-in-2"
// kaydi sig bir dugumde "mat-in-2" gibi okunur ve motor olmayan bir mat
// gorur.
static inline int valueToTT(int v, int ply) {
    if (v > MATE_THRESHOLD) return v + ply;
    if (v < -MATE_THRESHOLD) return v - ply;
    return v;
}
static inline int valueFromTT(int v, int ply) {
    if (v > MATE_THRESHOLD) return v - ply;
    if (v < -MATE_THRESHOLD) return v + ply;
    return v;
}

// sira kimdeyse onun sahi tehdit altinda mi?
// 64 kare taramasi yok; sah karesi ChessBoard tarafindan tutuluyor.
bool AI::isInCheck(ChessBoard& board) {
    return board.isKingAttacked(board.currentPlayer);
}

// TT boyutunu MB cinsinden ayarla (giris sayisi 2'nin kuvveti olur)
// ============================================================
//  SEVIYE SISTEMI
// ============================================================
void AI::setSkillLevel(int level) {
    if (level < 0) level = 0;
    if (level > 20) level = 20;
    skillLevel = level;
}

void AI::setHashSizeMB(int mb) {
    if (mb < 1) mb = 1;
    if (mb > MAX_HASH_MB) mb = MAX_HASH_MB;   // ilan edilen sinir uygulanmali
    size_t bytes = (size_t)mb * 1024u * 1024u;
    size_t want = bytes / sizeof(TTEntry);
    size_t pow2 = 1024;
    while (pow2 * 2 <= want) pow2 *= 2;
    ttEntries = pow2;
    ttMask = pow2 - 1;
    transpositionTable.assign(ttEntries, TTEntry());
}

int AI::hashFullPerMille() const {
    size_t n = (ttEntries < 1000) ? ttEntries : 1000;
    if (n == 0) return 0;
    int used = 0;
    for (size_t i = 0; i < n; i++)
        if (transpositionTable[i].depth != -1) used++;
    return (int)((used * 1000) / n);
}

void AI::clearTT() {
    // eleman eleman dolasmak yerine toplu doldurma.
    std::fill(transpositionTable.begin(), transpositionTable.end(), TTEntry());
}

void AI::clearHistory() {
    // 5 katli dongu yerine tek blok temizleme.
    std::memset(historyMoves, 0, sizeof(historyMoves));
    for (int i = 0; i < MAX_PLY; i++) { killerMoves[i][0] = Move(); killerMoves[i][1] = Move(); }
    for (int a = 0; a < 8; a++) for (int b = 0; b < 8; b++)
        for (int c = 0; c < 8; c++) for (int d = 0; d < 8; d++)
            counterMoveTable[a][b][c][d] = Move();
}

int AI::see(ChessBoard& board, Move m) {
    int targetVal = getPieceValue(board.getPiece(m.targetRow, m.targetCol));
    int attackerVal = getPieceValue(board.getPiece(m.startRow, m.startCol));

    if (m.promotionPiece != 0) {
        targetVal += getPieceValue(m.promotionPiece) - getPieceValue(1);
        attackerVal = getPieceValue(m.promotionPiece);
    }

    if (targetVal >= attackerVal) return targetVal;

    bool isDefended = board.isSquareAttacked(m.targetRow, m.targetCol, -board.currentPlayer);
    if (isDefended) {
        return targetVal - attackerVal;
    }
    return targetVal;
}

int AI::evaluateBoard(ChessBoard& board) {
    int mgScore = 0; int egScore = 0; int phase = 0;
    int whiteBishops = 0; int blackBishops = 0;

    int wKingR = -1, wKingC = -1;
    int bKingR = -1, bKingC = -1;

    // --- DUZELTME: PHASE ARTIK ON GECISTE HESAPLANIYOR ---
    // Onceki surumde phase ana dongunun ICINDE birikiyordu. Siyah sah
    // (satir 0) taramanin basinda kismi bir phase degeriyle, beyaz sah
    // (satir 7) neredeyse tam degerle degerlendiriliyordu. Sonuc: ayni
    // pozisyon iki renk icin farkli puanlaniyordu (mirror testinde 100
    // santipiyonluk sapma; baslangic pozisyonunda motor beyazi yarim
    // piyon geride saniyordu).
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int p = board.getPiece(r, c);
            if (p == 0) continue;
            if (p == W_KING) { wKingR = r; wKingC = c; }
            else if (p == B_KING) { bKingR = r; bKingC = c; }

            int t = std::abs(p);
            if (t == W_KNIGHT || t == W_BISHOP) phase += 1;
            else if (t == W_ROOK) phase += 2;
            else if (t == W_QUEEN) phase += 4;
        }
    }
    if (phase > 24) phase = 24;

    int knightMoves[8][2] = { {-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1} };
    int slidingMoves[8][2] = { {-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1} };

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.getPiece(r, c);
            if (piece == 0) continue;

            int type = std::abs(piece);
            int mgVal = 0, egVal = 0;
            int tableRow = (piece > 0) ? r : 7 - r;

            int tropismBonus = 0;
            if (type == 2 || type == 4 || type == 5) {
                int enemyKingR = (piece > 0) ? bKingR : wKingR;
                int enemyKingC = (piece > 0) ? bKingC : wKingC;

                if (enemyKingR != -1 && enemyKingC != -1) {
                    int distance = std::abs(r - enemyKingR) + std::abs(c - enemyKingC);
                    int proximity = 14 - distance;
                    if (type == 2) tropismBonus = proximity * 2;
                    else if (type == 4) tropismBonus = proximity * 3;
                    else if (type == 5) tropismBonus = proximity * 6;
                }
            }

            int mobilityBonus = 0;
            int threatBonus = 0;

            if (type == 2) {
                int mob = 0;
                for (int i = 0; i < 8; i++) {
                    int nr = r + knightMoves[i][0];
                    int nc = c + knightMoves[i][1];
                    if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                        int target = board.getPiece(nr, nc);
                        if (target == 0 || (piece > 0 ? target < 0 : target > 0)) {
                            mob++;
                            if (target != 0) {
                                int tType = std::abs(target);
                                if (tType == 4) threatBonus += 20;
                                if (tType == 5) threatBonus += 40;
                            }
                        }
                    }
                }
                if (mob == 0) mobilityBonus -= 30;
                else mobilityBonus += (mob * 4);
            }
            else if (type == 3 || type == 4 || type == 5) {
                int mob = 0;
                int startIdx = (type == 4) ? 4 : 0;
                int endIdx = (type == 3) ? 3 : 7;

                for (int i = startIdx; i <= endIdx; i++) {
                    int dr = slidingMoves[i][0];
                    int dc = slidingMoves[i][1];
                    int nr = r + dr, nc = c + dc;
                    while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                        int target = board.getPiece(nr, nc);
                        if (target == 0) { mob++; }
                        else {
                            if (piece > 0 ? target < 0 : target > 0) {
                                mob++;
                                int tType = std::abs(target);
                                if (type == 3 && tType == 4) threatBonus += 20;
                                if ((type == 3 || type == 4) && tType == 5) threatBonus += 40;
                            }
                            break;
                        }
                        nr += dr; nc += dc;
                    }
                }
                if (type == 3) { if (mob == 0) mobilityBonus -= 30; else mobilityBonus += (mob * 3); }
                else if (type == 4) { if (mob == 0) mobilityBonus -= 15; else mobilityBonus += (mob * 2); }
                else if (type == 5) { mobilityBonus += (mob * 1); }
            }

            int xrayBonus = 0;
            if (type == 4) {
                int enemyQueen = (piece > 0) ? -5 : 5;
                int enemyKing = (piece > 0) ? -6 : 6;
                bool xrayDetected = false;
                for (int i = 0; i < 8; i++) {
                    if (board.getPiece(i, c) == enemyQueen || board.getPiece(i, c) == enemyKing) { xrayDetected = true; break; }
                }
                if (!xrayDetected) {
                    for (int i = 0; i < 8; i++) {
                        if (board.getPiece(r, i) == enemyQueen || board.getPiece(r, i) == enemyKing) { xrayDetected = true; break; }
                    }
                }
                if (xrayDetected) xrayBonus = 20;
            }

            int fianchettoBonus = 0;
            if (type == 3) {
                if (piece > 0 && ((r == 6 && c == 1) || (r == 6 && c == 6))) fianchettoBonus = 30;
                else if (piece < 0 && ((r == 1 && c == 1) || (r == 1 && c == 6))) fianchettoBonus = 30;

                // YENI: Sikismis Tas (Trapped Bishop) Cezasi
                if (piece > 0) {
                    if (r == 1 && c == 0 && board.getPiece(2, 1) == -1) fianchettoBonus -= 50;
                    if (r == 1 && c == 7 && board.getPiece(2, 6) == -1) fianchettoBonus -= 50;
                }
                else {
                    if (r == 6 && c == 0 && board.getPiece(5, 1) == 1) fianchettoBonus -= 50;
                    if (r == 6 && c == 7 && board.getPiece(5, 6) == 1) fianchettoBonus -= 50;
                }
            }

            int outpostBonus = 0;
            if (type == 2) {
                if (piece > 0 && r >= 2 && r <= 4) {
                    if ((r < 7 && c > 0 && board.getPiece(r + 1, c - 1) == 1) || (r < 7 && c < 7 && board.getPiece(r + 1, c + 1) == 1)) {
                        outpostBonus = 25;
                    }
                }
                else if (piece < 0 && r >= 3 && r <= 5) {
                    if ((r > 0 && c > 0 && board.getPiece(r - 1, c - 1) == -1) || (r > 0 && c < 7 && board.getPiece(r - 1, c + 1) == -1)) {
                        outpostBonus = 25;
                    }
                }
            }

            int openFileBonus = 0;
            if (type == 4) {
                bool hasOwnPawn = false;
                bool hasEnemyPawn = false;
                int myPawn = (piece > 0) ? 1 : -1;
                int enemyPawn = (piece > 0) ? -1 : 1;
                for (int i = 0; i < 8; i++) {
                    if (board.getPiece(i, c) == myPawn) hasOwnPawn = true;
                    if (board.getPiece(i, c) == enemyPawn) hasEnemyPawn = true;
                }
                if (!hasOwnPawn && !hasEnemyPawn) openFileBonus = 25;
                else if (!hasOwnPawn && hasEnemyPawn) openFileBonus = 12;

                // YENI: 7. Yataydaki Kale (Rook on 7th Rank)
                if (piece > 0 && r == 1) openFileBonus += 25;
                else if (piece < 0 && r == 6) openFileBonus += 25;
            }

            if (type == 1) {
                mgVal = mg_value[1] + mg_pawnTable[tableRow][c];
                egVal = eg_value[1] + eg_pawnTable[tableRow][c];

                bool isIsolated = true, isDoubled = false, isPassed = true;
                bool isConnected = false, isPhalanx = false;
                int spaceBonus = 0;

                int forwardDir = (piece > 0) ? -1 : 1;
                int pR = r + forwardDir;
                int enemyColor = (piece > 0) ? -1 : 1;

                if (pR >= 0 && pR < 8) {
                    int attackCols[] = { c - 1, c + 1 };
                    for (int aCol : attackCols) {
                        if (aCol >= 0 && aCol < 8) {
                            int target = board.getPiece(pR, aCol);
                            if (target != 0 && (target * enemyColor > 0)) {
                                int tType = std::abs(target);
                                if (tType == 2 || tType == 3) threatBonus += 25;
                                else if (tType == 4) threatBonus += 35;
                                else if (tType == 5) threatBonus += 50;
                            }
                        }
                    }
                }

                for (int i = 0; i < 8; i++) { if (i != r && board.getPiece(i, c) == piece) isDoubled = true; }
                for (int i = 0; i < 8; i++) {
                    if (c > 0 && board.getPiece(i, c - 1) == piece) isIsolated = false;
                    if (c < 7 && board.getPiece(i, c + 1) == piece) isIsolated = false;
                }

                int enemyPawn = (piece > 0) ? -1 : 1;
                int startR = r + forwardDir;
                while (startR >= 0 && startR < 8) {
                    if (board.getPiece(startR, c) == enemyPawn) isPassed = false;
                    if (c > 0 && board.getPiece(startR, c - 1) == enemyPawn) isPassed = false;
                    if (c < 7 && board.getPiece(startR, c + 1) == enemyPawn) isPassed = false;
                    startR += forwardDir;
                }

                if (piece > 0) {
                    if (r < 7 && c > 0 && board.getPiece(r + 1, c - 1) == 1) isConnected = true;
                    if (r < 7 && c < 7 && board.getPiece(r + 1, c + 1) == 1) isConnected = true;
                    if (c > 0 && board.getPiece(r, c - 1) == 1) isPhalanx = true;
                    if (c < 7 && board.getPiece(r, c + 1) == 1) isPhalanx = true;
                    if ((c >= 2 && c <= 5) && (r == 3 || r == 4)) spaceBonus += 15;
                }
                else {
                    if (r > 0 && c > 0 && board.getPiece(r - 1, c - 1) == -1) isConnected = true;
                    if (r > 0 && c < 7 && board.getPiece(r - 1, c + 1) == -1) isConnected = true;
                    if (c > 0 && board.getPiece(r, c - 1) == -1) isPhalanx = true;
                    if (c < 7 && board.getPiece(r, c + 1) == -1) isPhalanx = true;
                    if ((c >= 2 && c <= 5) && (r == 4 || r == 3)) spaceBonus += 15;
                }

                if (isDoubled) { mgVal -= 11; egVal -= 11; }
                if (isIsolated) { mgVal -= 15; egVal -= 15; }
                if (isConnected) { mgVal += 15; egVal += 10; }
                if (isPhalanx) { mgVal += 12; egVal += 8; }

                mgVal += spaceBonus;

                if (isPassed) {
                    int rank = (piece > 0) ? (6 - r) : (r - 1);
                    int passedBonus = 15 + (rank * rank * 10);
                    mgVal += passedBonus;
                    egVal += passedBonus + (rank * 10);
                }
            }
            else if (type == 2) {
                mgVal = mg_value[2] + mg_knightTable[tableRow][c] + tropismBonus + mobilityBonus + outpostBonus + threatBonus;
                egVal = eg_value[2] + eg_knightTable[tableRow][c] + tropismBonus + mobilityBonus + outpostBonus + threatBonus;
            }
            else if (type == 3) {
                mgVal = mg_value[3] + mg_bishopTable[tableRow][c] + mobilityBonus + fianchettoBonus + threatBonus;
                egVal = eg_value[3] + eg_bishopTable[tableRow][c] + mobilityBonus + fianchettoBonus + threatBonus;
                if (piece > 0) whiteBishops++; else blackBishops++;
            }
            else if (type == 4) {
                mgVal = mg_value[4] + mg_rookTable[tableRow][c] + tropismBonus + mobilityBonus + xrayBonus + openFileBonus + threatBonus;
                egVal = eg_value[4] + eg_rookTable[tableRow][c] + tropismBonus + mobilityBonus + xrayBonus + openFileBonus + threatBonus;
            }
            else if (type == 5) {
                mgVal = mg_value[5] + mg_queenTable[tableRow][c] + tropismBonus + mobilityBonus;
                egVal = eg_value[5] + eg_queenTable[tableRow][c] + tropismBonus + mobilityBonus;
            }
            else if (type == 6) {
                mgVal = mg_value[6] + mg_kingTable[tableRow][c];
                egVal = eg_value[6] + eg_kingTable[tableRow][c];

                int kingSafetyBonus = 0;
                bool isWhite = (piece > 0);

                if (phase > 8) {
                    if (c >= 3 && c <= 5) {
                        kingSafetyBonus -= 50;
                    }
                    if (c <= 2 || c >= 6) {
                        kingSafetyBonus += 40;
                    }
                }

                int myPawn = isWhite ? 1 : -1;
                int shieldRow = isWhite ? r - 1 : r + 1;
                if (shieldRow >= 0 && shieldRow < 8) {
                    if (c >= 5) {
                        if (board.getPiece(shieldRow, 5) != myPawn) kingSafetyBonus -= 25;
                        if (board.getPiece(shieldRow, 6) != myPawn) kingSafetyBonus -= 35;
                        if (board.getPiece(shieldRow, 7) != myPawn) kingSafetyBonus -= 25;
                    }
                    else if (c <= 2) {
                        if (board.getPiece(shieldRow, 0) != myPawn) kingSafetyBonus -= 20;
                        if (board.getPiece(shieldRow, 1) != myPawn) kingSafetyBonus -= 30;
                        if (board.getPiece(shieldRow, 2) != myPawn) kingSafetyBonus -= 20;
                    }
                }

                int kingDangerPenalty = 0;
                int enemyColor = isWhite ? -1 : 1;

                for (int ir = std::max(0, r - 2); ir <= std::min(7, r + 2); ir++) {
                    for (int ic = std::max(0, c - 2); ic <= std::min(7, c + 2); ic++) {
                        int target = board.getPiece(ir, ic);
                        if (target != 0 && (target * enemyColor > 0)) {
                            int tType = std::abs(target);
                            if (tType == 5) kingDangerPenalty -= 90;
                            else if (tType == 4) kingDangerPenalty -= 40;
                            else if (tType == 2 || tType == 3) kingDangerPenalty -= 25;
                            else if (tType == 1) kingDangerPenalty -= 15;
                        }
                    }
                }

                mgVal += kingSafetyBonus + kingDangerPenalty;
            }

            if (piece > 0) { mgScore += mgVal; egScore += egVal; }
            else { mgScore -= mgVal; egScore -= egVal; }
        }
    }

    if (whiteBishops >= 2) { mgScore += 30; egScore += 45; }
    if (blackBishops >= 2) { mgScore -= 30; egScore -= 45; }

    int finalScore = (mgScore * phase + egScore * (24 - phase)) / 24;

    // (Iki dal ayni islemi yapiyordu; tek satira indirildi - davranis ayni.)
    finalScore += (finalScore * (24 - phase)) / 100;

    if (board.currentPlayer == 1) finalScore += 15;
    else finalScore -= 15;

    return finalScore;
}

int AI::getPieceValue(int pieceType) {
    int type = std::abs(pieceType);
    if (type >= 1 && type <= 5) return mg_value[type];
    return 0;
}

// ============================================================
//  HAMLE SIRALAMA YENIDEN YAZILDI
// ============================================================
// Onceki surumde skorlama std::sort'un KARSILASTIRMA FONKSIYONUNUN
// icindeydi. Bu iki agir sorun demekti:
//   1) Her karsilastirmada iki hamlenin skoru bastan hesaplaniyordu ve
//      skorlama see() cagiriyor, o da isSquareAttacked ile tum tahtayi
//      tariyordu. Yani dugum basina O(n log n) x 2 tahta taramasi.
//   2) "a == pvMove -> return true" ifadesi comp(a,a) icin de true
//      donuyordu; bu strict weak ordering ihlalidir ve teknik olarak
//      tanimsiz davranistir.
//
// Artik skorlar hamle basina BIR KEZ hesaplaniyor, sonra siralaniyor.
// Esitlik durumunda ozel bir kural YOK; hangi hamlenin one gectigi
// std::sort'un o derleyicideki uygulamasina bagli. Bu yuzden bench
// dugum sayilari ayni derleyici icinde tekrarlanabilir, farkli
// derleyiciler arasinda degil. Gerekcesi 625. satirda.
void AI::orderMoves(std::vector<Move>& moves, ChessBoard& board, int ply, Move pvMove, Move prevMove) {
    int n = (int)moves.size();
    if (n < 2) return;
    if (n > MAX_MOVES) n = MAX_MOVES;

    int playerIndex = (board.currentPlayer == 1) ? 0 : 1;
    int kIdx = (ply >= 0 && ply < MAX_PLY) ? ply : 0;

    Move counterM = Move();
    if (prevMove.startRow != -1) {
        counterM = counterMoveTable[prevMove.startRow][prevMove.startCol][prevMove.targetRow][prevMove.targetCol];
    }

    for (int i = 0; i < n; i++) {
        const Move& m = moves[i];
        int score;

        if (m == pvMove) {
            score = 2000000;   // her zaman en onde
        }
        else {
            int target = std::abs(board.getPiece(m.targetRow, m.targetCol));
            if (target != 0) {
                if (see(board, m) < 0) {
                    score = 10000 + target;                  // kaybettiren alim: killer'larin altinda
                }
                else {
                    int moving = std::abs(board.getPiece(m.startRow, m.startCol));
                    score = 100000 + 10 * getPieceValue(target) - getPieceValue(moving);
                }
            }
            else if (m == killerMoves[kIdx][0]) score = 90000;
            else if (m == killerMoves[kIdx][1]) score = 80000;
            else if (m == counterM)             score = 70000;
            else {
                // history degeri her kesmede depth*depth kadar
                // buyuyor ve tavani yoktu; uzun bir aramada 70000'i asip
                // killer/counter bandinin ustune cikabiliyordu, yani
                // siralama bantlari birbirine giriyordu. Tavan eklendi.
                int h = historyMoves[playerIndex][m.startRow][m.startCol][m.targetRow][m.targetCol];
                score = (h > 60000) ? 60000 : h;
            }
            if (m.promotionPiece != 0) score += 500000;
        }

        orderScratch[i].score = score;
        orderScratch[i].index = i;
    }

    // NOT: esitlikte belirleyici bir kural (ornegin uretim sirasi) eklemeyi
    // denedik - siralamayi derleyiciden bagimsiz yapiyordu ama olctugumuzde
    // ayni derinlikte %13 daha fazla dugum gerektirdi. Determinizm bu bedele
    // degmedi. Karsilastirici yine de gecerli bir strict weak ordering;
    // eski surumdeki "comp(a,a) == true" tanimsiz davranisi ortadan kalkti.
    std::sort(orderScratch, orderScratch + n, [](const OrderEntry& a, const OrderEntry& b) {
        return a.score > b.score;
        });

    for (int i = 0; i < n; i++) moveScratch[i] = moves[orderScratch[i].index];
    for (int i = 0; i < n; i++) moves[i] = moveScratch[i];
}

// ============================================================
//  NEGAMAX
// ============================================================
// Skorlar artik "sira kimdeyse ONA gore". Beyaz icin +100 ne demekse
// siyah icin de +100 ayni seyi ifade eder: "benim lehime bir piyon".
// Boylece maksimize/minimize eden iki ayri kol tamamen ortadan kalkti.
// Her rekursif cagri tek bicimde:   -search(..., -beta, -alpha)
//
// evaluateBoard hala BEYAZA gore puanliyor (mirror testi buna dayaniyor);
// donusum tek noktada yapiliyor.
int AI::evalSTM(ChessBoard& board) {
    int e = evaluateBoard(board);
    return (board.currentPlayer == 1) ? e : -e;
}

int AI::evaluateQuiet(ChessBoard& board) {
    // qsearch her 1024 dugumde bir sure kontrolu yapiyor ve timeLimitMs'e
    // bakiyor. Burasi arama disindan cagrildigi icin o alanlar bir onceki
    // aramadan kalma olabilir; guvenli bir pencere kurup geri koyuyoruz.
    auto      savedStart = startTime;
    long long savedLimit = timeLimitMs;
    bool      savedStop  = stopSearch;
    long long savedNodes = nodes;

    startTime   = std::chrono::steady_clock::now();
    timeLimitMs = 1000;   // pratikte asilmaz; qsearch mikrosaniyeler suruyor
    stopSearch  = false;

    int v = qsearch(board, -INFINITE_SCORE, INFINITE_SCORE, 0, 0);

    startTime   = savedStart;
    timeLimitMs = savedLimit;
    stopSearch  = savedStop;
    nodes       = savedNodes;   // lastNodes() raporunu bozmayalim
    return v;
}

int AI::qsearch(ChessBoard& board, int alpha, int beta, int ply, int qsDepth) {
    nodes++;
    if (ply > selDepth) selDepth = ply;

    // quiescence'ta daha once HIC zaman kontrolu yoktu; patolojik
    // bir pozisyonda sure limitini asabiliyordu.
    if ((nodes & 1023) == 0) {
        if (abortSearch.load(std::memory_order_relaxed)) { stopSearch = true; return 0; }
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > timeLimitMs) {
            stopSearch = true; return 0;
        }
    }

    if (board.halfMoveClock >= 100 || board.isInsufficientMaterial()) return 0;
    if (ply >= MAX_PLY - 1) return evalSTM(board);

    // Mutlak tavan: bitmeyen sah zincirlerine karsi guvenlik freni.
    if (qsDepth > 10) return evalSTM(board);

    bool inCheck = isInCheck(board);
    int best;

    // ============================================================
    //  SAH ALTINDA ARTIK STAND-PAT YOK
    // ============================================================
    // Onceki surum sah cekilmisken de "durup" statik degerlendirmeyi
    // dondurebiliyordu. Ama sahtayken hicbir sey oldugu yerde kalmaz:
    // oynamak ZORUNDASINIZ ve elinizdeki tek hamleler taviz olabilir.
    // Statik skor bu yuzden gercek degeri fena halde yaniltiyordu -
    // Stockfish macindaki taktik korlugun ana kaynagi buydu.
    // Artik sahtayken durmuyoruz, butun kacislari ariyoruz.
    if (inCheck) {
        best = -INFINITE_SCORE;
    }
    else {
        int stand = evalSTM(board);
        if (qsDepth > 6) return stand;
        if (stand >= beta) return beta;
        if (stand > alpha) alpha = stand;
        best = stand;
    }

    // sahtayken tum kacislar gerekli; degilse sadece alim/terfi
    // uretiyoruz. Filtre artik make/undo'dan ONCE uygulandigi icin
    // kullanilmayacak hamleler icin legallik kontrolu hic yapilmiyor.
    //
    // NOT: alim-only uretimde "hamle kalmadi" bilgisi pat anlamina gelmez,
    // bu yuzden pat tespiti sadece sah altindaki dala kaldi. Sahtayken mat
    // tespiti korunuyor; pat ise ust seviyedeki search dugumlerinde zaten
    // dogru yakalaniyor (bu, motorlarda standart davranistir).
    // ply tamponuna uret, heap tahsisi yok.
    std::vector<Move>& moves = moveBuf[ply];
    if (inCheck) moveGen.getLegalMoves(board, moves);
    else         moveGen.getLegalCaptures(board, moves);

    if (moves.empty()) {
        if (inCheck) return -(MATE_VALUE - ply);   // MAT
        return best;                               // arayacak alim yok
    }

    orderMoves(moves, board, ply, Move(), Move());

    for (const Move& m : moves) {
        if (!inCheck && see(board, m) < 0) continue;

        board.makeMove(m);
        int score = -qsearch(board, -beta, -alpha, ply + 1, qsDepth + 1);
        board.undoMove();

        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }
    return best;
}

int AI::search(ChessBoard& board, int depth, int ply, int alpha, int beta,
    bool allowNullMove, Move prevMove, Move excludedMove) {
    nodes++;
    unsigned long long boardKey = board.zobristKey;

    // Bu dugum icin PV'yi sifirla. Asagidaki erken donuslerin hepsi
    //boylece "hat yok" birakiyor; bayat veri yukari kopyalanmiyor.
    pvLength[ply] = 0;
    if (ply > selDepth) selDepth = ply;
    // TEKRAR TARAMASI SINIRLANDI
    // Onceki surum her dugumde tum gecmisi bastan sona tariyordu; 60 hamlelik
    // bir macta dugum basina ~120 karsilastirma. Geri donusu olmayan son
    // hamleden (halfMoveClock) oteye bakmanin anlami yok, ayrica sadece ayni
    // tarafin oynadigi pozisyonlar tekrar olabilecegi icin 2 adim atlaniyor.
    if (excludedMove.startRow == -1) {
        int n = (int)searchHistory.size();
        int limit = board.halfMoveClock;
        if (limit > n) limit = n;
        for (int i = n - 2; i >= n - limit && i >= 0; i -= 2) {
            if (searchHistory[i] == boardKey) return 0;
        }
    }

    if (ply > 0 && (board.halfMoveClock >= 100 || board.isInsufficientMaterial())) return 0;
    if (ply >= MAX_PLY - 1) return evalSTM(board);

    if (depth <= 0) return qsearch(board, alpha, beta, ply, 0);

    // zaman kontrolu her dugumde degil, 1024 dugumde bir.
    // steady_clock::now() dugum basina ~20 ns; bu sikta cagrilmasi olculebilir
    // bir maliyetti. Ayni yerde harici durdurma bayragi da kontrol ediliyor.
    if ((nodes & 1023) == 0) {
        if (abortSearch.load(std::memory_order_relaxed)) { stopSearch = true; return 0; }
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > timeLimitMs) {
            stopSearch = true; return 0;
        }
    }

    // ---------------- TRANSPOZISYON TABLOSU ----------------
    size_t ttIndex = (size_t)(boardKey & ttMask);   // 2'nin kuvveti -> maske
    TTEntry ttEntry = transpositionTable[ttIndex];   // referans degil, deger kopyasi
    Move ttMove;
    int ttValue = 0;
    bool hasValidTT = false;

    // PV dugumu: pencere genis (beta - alpha > 1). Bu dugumlerde hazir
    // TT cevabi ALMIYORUZ; alsaydik beklenen hat oradan kesilirdi.
    const bool isPvNode = (beta - alpha > 1);

    if (ttEntry.key == boardKey) {
        ttMove = ttEntry.bestMove;
        ttValue = valueFromTT(ttEntry.value, ply);
        hasValidTT = true;
        if (excludedMove.startRow == -1 && !isPvNode && ttEntry.depth >= depth) {
            if (ttEntry.flags == HASH_EXACT) return ttValue;
            if (ttEntry.flags == HASH_ALPHA && ttValue <= alpha) return alpha;
            if (ttEntry.flags == HASH_BETA && ttValue >= beta)  return beta;
        }
    }

    bool inCheck = isInCheck(board);

    // ---------------- BOS HAMLE (NULL MOVE) ----------------
    // Negamax'ta "beyaz beta'yi asti" ve "siyah alpha'nin altina indi"
    // kosullari tek bir ifadeye indi: evalSTM >= beta.
    // DUGUM BASINA TEK STATIK DEGERLENDIRME
    // Onceki surumde bos hamle, razoring ve futility bloklari evalSTM'i
    // AYRI AYRI cagiriyordu; derinlik 3'te ayni dugumde uc kez tam
    // degerlendirme yapiliyordu. Artik en fazla bir kez hesaplaniyor.
    bool needStatic = (!inCheck && excludedMove.startRow == -1) &&
        ((allowNullMove && depth >= 3) || depth <= 3);
    int staticEval = needStatic ? evalSTM(board) : 0;

    if (allowNullMove && depth >= 3 && !inCheck && excludedMove.startRow == -1) {
        int eval = staticEval;
        int R = (depth > 6) ? 3 : 2;

        if (eval >= beta) {
            int epRowBackup = board.enPassantRow;
            int epColBackup = board.enPassantCol;
            unsigned long long keyBackup = board.zobristKey;

            board.enPassantRow = -1; board.enPassantCol = -1;
            board.currentPlayer *= -1;
            board.generateHash();

            int nullScore = -search(board, depth - 1 - R, ply + 1, -beta, -beta + 1, false, Move(), Move());

            board.currentPlayer *= -1;
            board.enPassantRow = epRowBackup; board.enPassantCol = epColBackup;
            board.zobristKey = keyBackup;

            if (nullScore >= beta) return nullScore;
        }
    }

    // ---------------- RAZORING ----------------
    if (depth <= 3 && !inCheck && excludedMove.startRow == -1) {
        int razorMargin = 300 + (depth - 1) * 100;

        if (staticEval + razorMargin <= alpha) {
            int qScore = qsearch(board, alpha, beta, ply, 0);
            if (qScore <= alpha) return qScore;
        }
    }

    // ---------------- FUTILITY ----------------
    bool fPrune = false;
    int fMargin = 250 * depth;
    if (depth <= 3 && !inCheck && excludedMove.startRow == -1) {
        if (staticEval + fMargin <= alpha) fPrune = true;
    }

    // ---------------- SINGULAR EXTENSION ----------------
    // Onceki surumde bayrak kontrolu "maksimize eden icin ALPHA, minimize
    // eden icin BETA" idi; negamax'ta ikisi de ALPHA'ya (ust sinir) denk
    // geliyor, cunku artik her dugum kendi acisindan maksimize ediyor.
    bool isSingular = false;
    int singularExtension = 0;

    if (depth >= 4 && hasValidTT && ttMove.startRow != -1 && excludedMove.startRow == -1 && !inCheck) {
        if (ttEntry.flags == HASH_EXACT || ttEntry.flags == HASH_ALPHA) {
            int rDepth = (depth - 1) / 2;
            int rBeta = ttValue - 50;

            int seScore = search(board, rDepth, ply, rBeta - 1, rBeta, false, prevMove, ttMove);

            if (seScore < rBeta) {
                isSingular = true;
                singularExtension = 1;
            }
        }
        // Singular arama AYNI ply ile cagriliyor ve pvTable[ply]'i
        // kirletmis olabilir; kendi hamle dongumuze temiz basliyoruz.
        pvLength[ply] = 0;
    }

    // ply tamponuna uret, heap tahsisi yok.
    std::vector<Move>& moves = moveBuf[ply];
    moveGen.getLegalMoves(board, moves);

    if (moves.empty()) {
        if (!inCheck) return 0;                 // PAT
        return -(MATE_VALUE - ply);             // MAT (ply ne kadar kucukse o kadar kotu)
    }

    orderMoves(moves, board, ply, ttMove.startRow != -1 ? ttMove : Move(), prevMove);
    if (excludedMove.startRow == -1) searchHistory.push_back(boardKey);

    // killer'lar artik PLY ile indeksleniyor.
    // Derinlikle indekslemek uzatmali varyantlarda anlamsizdi: ayni
    // derinlikteki ama bambaska ply'lardaki dugumler ayni killer
    // kutusunu paylasiyordu. Killer sezgisi "AYNI seviyedeki kardes
    // dugumlerde ise yarayan hamle" fikrine dayanir, o da ply'dir.
    const int kIdx = (ply >= 0 && ply < MAX_PLY) ? ply : 0;

    bool firstMove = true; int movesSearched = 0;
    int originalAlpha = alpha;
    int bestEval = -INFINITE_SCORE;
    Move bestMoveInNode;

    for (const Move& m : moves) {
        if (m == excludedMove) continue;

        bool isQuiet = (board.getPiece(m.targetRow, m.targetCol) == 0 && m.promotionPiece == 0);
        if (fPrune && isQuiet && !firstMove && !isSingular) continue;

        board.makeMove(m);

        // makeMove sonrasi sira rakipte; isInCheck "sah cektik mi" sorusunu yanitlar.
        //
        // UZATMA TAVANI. "depth - 1 + extension" ifadesi derinligi
        // hic azaltmadigi icin surekli sah ceken varyantlar nominal derinligi
        // asip zaman butcesini yiyordu. Tekrar tespiti bunu sinirliyordu ama
        // garanti degildi. Artik kokun iki katindan derine uzatma yok.
        int extension = 0;
        if (ply < 2 * rootDepth) {
            if (isInCheck(board)) extension = 1;
            if (isSingular && m == ttMove) extension = std::max(extension, singularExtension);
        }

        int eval;

        if (firstMove) {
            eval = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, true, m, Move());
            firstMove = false;
        }
        else if (depth >= 3 && movesSearched >= 4 && isQuiet && extension == 0) {
            // LMR: once indirimli ve dar pencereyle dene
            eval = -search(board, depth - 2, ply + 1, -alpha - 1, -alpha, true, m, Move());
            if (eval > alpha) {
                eval = -search(board, depth - 1 + extension, ply + 1, -alpha - 1, -alpha, true, m, Move());
                if (eval > alpha && eval < beta)
                    eval = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, true, m, Move());
            }
        }
        else {
            // PVS: dar pencere, tutmazsa tam pencere
            eval = -search(board, depth - 1 + extension, ply + 1, -alpha - 1, -alpha, true, m, Move());
            if (eval > alpha && eval < beta)
                eval = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, true, m, Move());
        }

        board.undoMove(); movesSearched++;
        if (stopSearch) { if (excludedMove.startRow == -1) searchHistory.pop_back(); return 0; }

        if (eval > bestEval) { bestEval = eval; bestMoveInNode = m; }
        
            if (eval > alpha) {
            alpha = eval;
            
                // PV KAYDI: bu hamle + cocuk dugumun hatti.
                pvTable[ply][0] = m;
            int childLen = (ply + 1 < MAX_PLY) ? pvLength[ply + 1] : 0;
            if (childLen > MAX_PV - 1) childLen = MAX_PV - 1;
            for (int i = 0; i < childLen; i++) pvTable[ply][i + 1] = pvTable[ply + 1][i];
            pvLength[ply] = childLen + 1;
        }

        if (alpha >= beta) {
            if (isQuiet && excludedMove.startRow == -1) {
                killerMoves[kIdx][1] = killerMoves[kIdx][0]; killerMoves[kIdx][0] = m;
                int pIndex = (board.currentPlayer == 1) ? 0 : 1;
                historyMoves[pIndex][m.startRow][m.startCol][m.targetRow][m.targetCol] += depth * depth;
                if (prevMove.startRow != -1) counterMoveTable[prevMove.startRow][prevMove.startCol][prevMove.targetRow][prevMove.targetCol] = m;
            }
            break;
        }
    }

    if (excludedMove.startRow == -1) {
        TTEntry& slot = transpositionTable[ttIndex];
        slot.key = boardKey; slot.depth = (int8_t)depth; slot.bestMove = bestMoveInNode;
        slot.value = valueToTT(bestEval, ply);
        if (bestEval <= originalAlpha) slot.flags = HASH_ALPHA;
        else if (bestEval >= beta)     slot.flags = HASH_BETA;
        else                           slot.flags = HASH_EXACT;
        searchHistory.pop_back();
    }

    return bestEval;
}

Move AI::getBestMoveTimed(ChessBoard& board, int maxDepth, long long limitMs, long long maxLimitMs, std::vector<unsigned long long> gameHistory) {
    nodes = 0;
    abortSearch.store(false, std::memory_order_relaxed);

    // Sonuc tamponlari EN BASTA temizlenmeli: asagida kitap hamlesi,
    // mat/pat ve tek-hamle kisayollari icin erken donusler var. Aksi
    // halde bir onceki aramanin hatlari bayat veri olarak geri doner.
    lastPv.clear();
    rootLines.clear();
    lastRootLines.clear();
    selDepth = 0;

    if (useBook) {
        if (!book.isLoaded()) book.load(bookPath);   // dosya yalnizca bir kez okunur
        Move bookMove = book.getBookMove(board, moveGen);
        if (!bookMove.isNull()) {
            // Kitap hamlesinde arama yapilmadi; onceki aramanin skoru ve
            // derinligi ekranda bayat veri olarak kalmasin.
            lastScore = 0;
            lastDepth = 0;
            return bookMove;
        }
    }

    std::vector<Move> legalMoves = moveGen.getLegalMoves(board);
    if (legalMoves.empty()) return Move();

    // Analizde tek legal hamle olsa bile skorunu istiyoruz; kisayol
    // yalnizca oynarken gecerli.
    if (legalMoves.size() == 1 && !wideRoot) { lastScore = 0; return legalMoves[0]; }

    searchHistory = gameHistory;
    startTime = std::chrono::steady_clock::now();
    timeLimitMs = limitMs;
    stopSearch = false;

    for (int i = 0; i < MAX_PLY; i++) { killerMoves[i][0] = Move(); killerMoves[i][1] = Move(); }
    clearHistory();

    Move absoluteBestMove = legalMoves[0];
    Move bestMoveInDepth;

    // zayif seviyelerde derinligi sinirla.
    // Seviye 20 -> sinir yok. 0 -> derinlik 1.
    bool skillMode = (skillLevel < 20);
    if (skillMode) {
        int lvlDepth = 1 + (skillLevel * 9) / 20;   // 1 ... 10
        if (lvlDepth < maxDepth) maxDepth = lvlDepth;
    }
    rootScores.clear();
    std::vector<std::pair<int, Move>> lastCompletedScores;
    std::vector<Move> pvInDepth;   // su anki yinelemenin en iyi hatti
    // skor artik sira sahibine gore, tek yonlu.
    int previousScore = -INFINITE_SCORE;
    int olderScore = previousScore;

    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++) {

        rootDepth = currentDepth;   // uzatma tavani icin

        int alpha = -INFINITE_SCORE;
        int beta = INFINITE_SCORE;
        int delta = 50;
        int aspFailCount = 0;   // guvenlik sayaci

        // Mat skoru elimizdeyken dar pencere anlamsizdir, tam pencere kullan.
        if (currentDepth >= 3 && !wideRoot && std::abs(previousScore) < MATE_THRESHOLD) {
            alpha = std::max(-INFINITE_SCORE, previousScore - delta);
            beta = std::min(INFINITE_SCORE, previousScore + delta);
        }

        while (true) {
            int currentAlpha = alpha;
            int currentBeta = beta;
            int bestValue = -INFINITE_SCORE;
            bestMoveInDepth = legalMoves[0];

            orderMoves(legalMoves, board, 0, absoluteBestMove, Move());   // kok = ply 0

            rootScores.clear();
            rootLines.clear();

            for (const Move& m : legalMoves) {
                board.makeMove(m);
                int boardValue = -search(board, currentDepth - 1, 1, -currentBeta, -currentAlpha, true, m, Move());
                board.undoMove();

                if (stopSearch) break;

                rootScores.push_back(std::make_pair(boardValue, m));

                if (wideRoot) {
                    // Kok hamleleri ply 1'de araniyor; cocugun hatti pvTable[1].
                    RootLine rl;
                    rl.score = boardValue;
                    rl.move = m;
                    rl.pv.push_back(m);
                    for (int i = 0; i < pvLength[1] && (int)rl.pv.size() < MAX_PV; i++)
                        rl.pv.push_back(pvTable[1][i]);
                    rootLines.push_back(rl);
                }

                // kok de artik sadece maksimize ediyor.
                if (boardValue > bestValue) {
                    bestValue = boardValue; bestMoveInDepth = m;
                    
                    // Kok hamleleri ply 1'de araniyor; cocugun hatti pvTable[1].
                    pvInDepth.clear();
                    pvInDepth.push_back(m);
                 for (int i = 0; i < pvLength[1] && (int)pvInDepth.size() < MAX_PV; i++)
                 pvInDepth.push_back(pvTable[1][i]);
                }

                // seviye modunda alpha'yi YUKSELTMIYORUZ.
                // Normalde alpha yukseldikce sonraki hamleler sadece "daha
                // kotu" diye isaretlenir, gercek skorlari bilinmez. Seviye
                // secimi icin her hamlenin gercek skoru lazim; bu yuzden
                // tam pencereyle ariyoruz. Zayif seviyelerde derinlik zaten
                // dusuk oldugu icin maliyeti onemsiz.
                if (!skillMode && !wideRoot && bestValue > currentAlpha) currentAlpha = bestValue;
            }

            if (stopSearch) break;

            // ================= DUZELTME: ASPIRATION PENCERESI =================
            // Onceki surumde genisletilen sinir minimize eden taraf icin TERSTI:
            //   siyah oynarken "bestValue <= alpha" durumunda alpha yerine beta
            //   aciliyordu. beta zaten 1000000 oldugu icin pencere hic degismiyor,
            //   ayni arama sonsuza kadar tekrarlaniyordu. Zaman kontrolu devreye
            //   girene kadar donuyor, sonra derinlik 2'nin hamlesi oynaniyordu.
            //   Maclarda bunun imzasi netti: siyahin derinligi 10-11'den birden
            //   2-3'e cokup bir daha toparlanmiyordu.
            //
            // Dogrusu, pencere beyaza gore olan skor ekseninde tanimli oldugu icin
            // her iki taraf icin de aynidir: alttan tasarsa alpha, ustten tasarsa
            // beta genisletilir. Sinir zaten sonsuzdaysa tekrar denemenin anlami
            // yok - dongunun sonlanma garantisi buradan geliyor.
            if (currentDepth >= 3) {
                bool failLow = (bestValue <= alpha && alpha > -INFINITE_SCORE);
                bool failHigh = (bestValue >= beta && beta < INFINITE_SCORE);

                if ((failLow || failHigh) && aspFailCount < 3) {
                    aspFailCount++;
                    delta += delta / 2 + 25;
                    if (failLow)  alpha = std::max(-INFINITE_SCORE, bestValue - delta);
                    if (failHigh) beta = std::min(INFINITE_SCORE, bestValue + delta);
                    if (aspFailCount >= 2) { alpha = -INFINITE_SCORE; beta = INFINITE_SCORE; }
                    continue;
                }
            }

            olderScore = previousScore;
            previousScore = bestValue;
            lastCompletedScores = rootScores;
            lastPv = pvInDepth;
            lastRootLines = rootLines;
            break;
        }

        if (stopSearch) break;
        absoluteBestMove = bestMoveInDepth;
        // previousScore zaten motorun kendi acisindan; cevirmeye gerek yok.
        lastDepth = currentDepth;
        int displayScore = previousScore;
        lastScore = displayScore;   // testler icin
        {
            auto tNow = std::chrono::steady_clock::now();
            long long msElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tNow - startTime).count();
            long long nps = (msElapsed > 0) ? (nodes * 1000 / msElapsed) : 0;

            std::ostringstream line;
            line << "info depth " << currentDepth << " seldepth " << selDepth;
            if (std::abs(displayScore) > MATE_THRESHOLD) {
                int matePlies = MATE_VALUE - std::abs(displayScore);
                int mateMoves = (matePlies + 1) / 2;
                if (displayScore < 0) mateMoves = -mateMoves;
                line << " score mate " << mateMoves;
            }
            else {
                line << " score cp " << displayScore;
            }
            line << " nodes " << nodes << " time " << msElapsed << " nps " << nps
                << " hashfull " << hashFullPerMille()
            << " pv";
            if (pvInDepth.empty()) line << " " << Notation::toUci(absoluteBestMove);
            else for (size_t i = 0; i < pvInDepth.size(); i++)
            line << " " << Notation::toUci(pvInDepth[i]);
            info(line.str());   // stdout yerine geri cagirim
        }

        if (currentDepth >= 3) {
            auto now = std::chrono::steady_clock::now();
            long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

            // "skorum dustu" iki renk icin de ayni ifade.
            bool inCrisis = (previousScore < olderScore - 50);

            if (inCrisis && maxLimitMs > limitMs) {
                timeLimitMs = maxLimitMs;
            }
            else if (!inCrisis && elapsed > (timeLimitMs * 0.6)) {
                break;
            }
        }
    }

    // ============================================================
    //  SEVIYEYE GORE HAMLE SECIMI
    // ============================================================
    // Her kok hamlesinin skoruna rastgele bir "gurultu" ekleyip en yuksegi
    // seciyoruz. Boylece d santipiyon daha kotu bir hamle ancak kendi cekilisi
    // en iyininkini d kadar asarsa kazanir - yani fark buyudukce secilme
    // olasiligi hizla duser. Sabit bir marj icinde duzgun rastgele secmekten
    // cok daha iyi davraniyor: sakin pozisyonlarda cesitlilik korunuyor,
    // acik bir en-iyi hamle varken nadiren kaciriliyor.
    //
    // Gurultu seviyeyle KAREsel olarak buyuyor: seviye 19'da neredeyse sifir,
    // seviye 0'da 200 santipiyon. Boylece ust seviyeler birbirine yakin,
    // alt seviyeler belirgin sekilde zayif oluyor.
    lastSkillLoss = 0;
    if (skillMode && lastCompletedScores.size() > 1) {
        int gap = 20 - skillLevel;
        int noise = (gap * gap) / 2;   // 19 -> 0, 15 -> 12, 10 -> 50, 5 -> 112, 0 -> 200

        int best = lastCompletedScores[0].first;
        for (size_t i = 1; i < lastCompletedScores.size(); i++)
            if (lastCompletedScores[i].first > best) best = lastCompletedScores[i].first;

        // Mat gorduysek oyunu uzatmayalim; en iyisini oyna.
        if (noise > 0 && std::abs(best) < MATE_THRESHOLD) {
            std::uniform_int_distribution<int> dist(0, noise);
            int bestNoisy = -INFINITE_SCORE;
            int chosenScore = best;

            for (size_t i = 0; i < lastCompletedScores.size(); i++) {
                int noisy = lastCompletedScores[i].first + dist(skillRng);
                if (noisy > bestNoisy) {
                    bestNoisy = noisy;
                    absoluteBestMove = lastCompletedScores[i].second;
                    chosenScore = lastCompletedScores[i].first;
                }
            }
            lastSkillLoss = best - chosenScore;
        }
    }

    return absoluteBestMove;
}
