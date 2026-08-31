#include "MoveGen.hpp"
#include <cstdlib>
#include <cmath>

bool MoveGenerator::isWithinBoard(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

void MoveGenerator::getSliderMoves(int row, int col, const int* rowOffsets, const int* colOffsets,
    int dirCount, ChessBoard& board, std::vector<Move>& out) {
    int myPiece = board.getPiece(row, col);
    int myColor = (myPiece > 0) ? 1 : -1;

    for (int i = 0; i < dirCount; i++) {
        int targetRow = row + rowOffsets[i];
        int targetCol = col + colOffsets[i];

        while (isWithinBoard(targetRow, targetCol)) {
            int targetPiece = board.getPiece(targetRow, targetCol);

            if (targetPiece == 0) {
                out.push_back(Move(row, col, targetRow, targetCol));
            }
            else if (targetPiece * myColor < 0) {
                out.push_back(Move(row, col, targetRow, targetCol));
                break;
            }
            else {
                break;
            }
            targetRow += rowOffsets[i];
            targetCol += colOffsets[i];
        }
    }
}

void MoveGenerator::getKnightMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    static const int rowOffsets[] = { -2, -2, -1, -1,  1,  1,  2,  2 };
    static const int colOffsets[] = { -1,  1, -2,  2, -2,  2, -1,  1 };
    int myPiece = board.getPiece(row, col);
    int myColor = (myPiece > 0) ? 1 : -1;

    for (int i = 0; i < 8; i++) {
        int targetRow = row + rowOffsets[i];
        int targetCol = col + colOffsets[i];
        if (isWithinBoard(targetRow, targetCol)) {
            int targetPiece = board.getPiece(targetRow, targetCol);
            if (targetPiece == 0 || (targetPiece * myColor < 0)) {
                out.push_back(Move(row, col, targetRow, targetCol));
            }
        }
    }
}

void MoveGenerator::getRookMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    static const int rowOffsets[] = { -1,  1,  0,  0 };
    static const int colOffsets[] = { 0,  0, -1,  1 };
    getSliderMoves(row, col, rowOffsets, colOffsets, 4, board, out);
}

void MoveGenerator::getBishopMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    static const int rowOffsets[] = { -1, -1,  1,  1 };
    static const int colOffsets[] = { -1,  1, -1,  1 };
    getSliderMoves(row, col, rowOffsets, colOffsets, 4, board, out);
}

void MoveGenerator::getQueenMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    static const int rowOffsets[] = { -1,  1,  0,  0, -1, -1,  1,  1 };
    static const int colOffsets[] = { 0,  0, -1,  1, -1,  1, -1,  1 };
    getSliderMoves(row, col, rowOffsets, colOffsets, 8, board, out);
}

void MoveGenerator::getPawnMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    int myPiece = board.getPiece(row, col);
    int myColor = (myPiece > 0) ? 1 : -1;
    int direction = (myColor == 1) ? -1 : 1;
    int startRow = (myColor == 1) ? 6 : 1;
    int promotionRow = (myColor == 1) ? 0 : 7;

    auto addPawnMove = [&](int targetR, int targetC) {
        if (targetR == promotionRow) {
            out.push_back(Move(row, col, targetR, targetC, myColor * W_QUEEN));
            out.push_back(Move(row, col, targetR, targetC, myColor * W_ROOK));
            out.push_back(Move(row, col, targetR, targetC, myColor * W_BISHOP));
            out.push_back(Move(row, col, targetR, targetC, myColor * W_KNIGHT));
        }
        else {
            out.push_back(Move(row, col, targetR, targetC, 0));
        }
        };

    int forwardOne = row + direction;
    if (isWithinBoard(forwardOne, col) && board.getPiece(forwardOne, col) == 0) {
        addPawnMove(forwardOne, col);

        int forwardTwo = row + (2 * direction);
        if (row == startRow && board.getPiece(forwardTwo, col) == 0) {
            out.push_back(Move(row, col, forwardTwo, col, 0));
        }
    }

    int attackCols[] = { col - 1, col + 1 };
    for (int i = 0; i < 2; i++) {
        int targetCol = attackCols[i];
        if (isWithinBoard(forwardOne, targetCol)) {
            int targetPiece = board.getPiece(forwardOne, targetCol);

            if (targetPiece != 0 && (targetPiece * myColor < 0)) {
                addPawnMove(forwardOne, targetCol);
            }
            else if (forwardOne == board.enPassantRow && targetCol == board.enPassantCol) {
                addPawnMove(forwardOne, targetCol);
            }
        }
    }
}

void MoveGenerator::getKingMoves(int row, int col, ChessBoard& board, std::vector<Move>& out) {
    static const int rowOffsets[] = { -1, -1, -1,  0,  0,  1,  1,  1 };
    static const int colOffsets[] = { -1,  0,  1, -1,  1, -1,  0,  1 };
    int myPiece = board.getPiece(row, col);
    int myColor = (myPiece > 0) ? 1 : -1;

    for (int i = 0; i < 8; i++) {
        int targetRow = row + rowOffsets[i];
        int targetCol = col + colOffsets[i];

        if (isWithinBoard(targetRow, targetCol)) {
            int targetPiece = board.getPiece(targetRow, targetCol);
            if (targetPiece == 0 || (targetPiece * myColor < 0)) {
                out.push_back(Move(row, col, targetRow, targetCol, 0));
            }
        }
    }

    int attackerColor = board.currentPlayer * -1;

    if (board.currentPlayer == 1) {
        if (board.whiteCastleK && board.getPiece(7, 5) == 0 && board.getPiece(7, 6) == 0 && board.getPiece(7, 7) == W_ROOK) {
            if (!board.isSquareAttacked(7, 4, attackerColor) && !board.isSquareAttacked(7, 5, attackerColor)) {
                out.push_back(Move(7, 4, 7, 6, 0));
            }
        }
        if (board.whiteCastleQ && board.getPiece(7, 1) == 0 && board.getPiece(7, 2) == 0 && board.getPiece(7, 3) == 0 && board.getPiece(7, 0) == W_ROOK) {
            if (!board.isSquareAttacked(7, 4, attackerColor) && !board.isSquareAttacked(7, 3, attackerColor)) {
                out.push_back(Move(7, 4, 7, 2, 0));
            }
        }
    }
    else {
        if (board.blackCastleK && board.getPiece(0, 5) == 0 && board.getPiece(0, 6) == 0 && board.getPiece(0, 7) == B_ROOK) {
            if (!board.isSquareAttacked(0, 4, attackerColor) && !board.isSquareAttacked(0, 5, attackerColor)) {
                out.push_back(Move(0, 4, 0, 6, 0));
            }
        }
        if (board.blackCastleQ && board.getPiece(0, 1) == 0 && board.getPiece(0, 2) == 0 && board.getPiece(0, 3) == 0 && board.getPiece(0, 0) == B_ROOK) {
            if (!board.isSquareAttacked(0, 4, attackerColor) && !board.isSquareAttacked(0, 3, attackerColor)) {
                out.push_back(Move(0, 4, 0, 2, 0));
            }
        }
    }
}

// ============================================================
//  URETIM VE LEGALLIK
// ============================================================
void MoveGenerator::generatePseudoMoves(ChessBoard& board, std::vector<Move>& out) {
    out.clear();
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.getPiece(r, c);
            if (piece == 0) continue;
            if ((piece > 0) != (board.currentPlayer == 1)) continue;

            switch (std::abs(piece)) {
            case W_PAWN:   getPawnMoves(r, c, board, out);   break;
            case W_KNIGHT: getKnightMoves(r, c, board, out); break;
            case W_BISHOP: getBishopMoves(r, c, board, out); break;
            case W_ROOK:   getRookMoves(r, c, board, out);   break;
            case W_QUEEN:  getQueenMoves(r, c, board, out);  break;
            case W_KING:   getKingMoves(r, c, board, out);   break;
            }
        }
    }
}

void MoveGenerator::getLegalMoves(ChessBoard& board, std::vector<Move>& out) {
    generatePseudoMoves(board, pseudoScratch);
    out.clear();

    int turnColor = board.currentPlayer;
    for (size_t i = 0; i < pseudoScratch.size(); i++) {
        Move m = pseudoScratch[i];
        board.makeMove(m);
        if (!board.isKingAttacked(turnColor)) out.push_back(m);
        board.undoMove();
    }
}

void MoveGenerator::getLegalCaptures(ChessBoard& board, std::vector<Move>& out) {
    generatePseudoMoves(board, pseudoScratch);
    out.clear();

    int turnColor = board.currentPlayer;
    for (size_t i = 0; i < pseudoScratch.size(); i++) {
        Move m = pseudoScratch[i];

        // Ucuz filtre once: alim, en passant veya terfi degilse hic ugrasma.
        if (board.getPiece(m.targetRow, m.targetCol) == 0 && m.promotionPiece == 0) {
            bool isEnPassant = (std::abs(board.getPiece(m.startRow, m.startCol)) == W_PAWN &&
                m.targetRow == board.enPassantRow && m.targetCol == board.enPassantCol);
            if (!isEnPassant) continue;
        }

        board.makeMove(m);
        if (!board.isKingAttacked(turnColor)) out.push_back(m);
        board.undoMove();
    }
}

// --- Kolaylik sarmalayicilari (sicak yolda kullanilmaz) ---
std::vector<Move> MoveGenerator::getLegalMoves(ChessBoard& board) {
    std::vector<Move> out;
    getLegalMoves(board, out);
    return out;
}

std::vector<Move> MoveGenerator::getLegalCaptures(ChessBoard& board) {
    std::vector<Move> out;
    getLegalCaptures(board, out);
    return out;
}