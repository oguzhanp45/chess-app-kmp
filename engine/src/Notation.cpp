#include "Notation.hpp"
#include <cstdlib>
#include <vector>

namespace Notation {

    std::string toUci(const Move& m) {
        if (m.isNull()) return "0000";
        std::string s;
        s += (char)(m.startCol + 'a');
        s += std::to_string(8 - m.startRow);
        s += (char)(m.targetCol + 'a');
        s += std::to_string(8 - m.targetRow);

        if (m.promotionPiece != 0) {
            int p = std::abs((int)m.promotionPiece);
            if (p == W_QUEEN)       s += "q";
            else if (p == W_ROOK)   s += "r";
            else if (p == W_BISHOP) s += "b";
            else if (p == W_KNIGHT) s += "n";
        }
        return s;
    }

    bool fromUci(const std::string& text, ChessBoard& board, MoveGenerator& moveGen, Move& out) {
        if (text.length() < 4) return false;

        int startCol = text[0] - 'a';
        int startRow = 8 - (text[1] - '0');
        int targetCol = text[2] - 'a';
        int targetRow = 8 - (text[3] - '0');

        if (startCol < 0 || startCol  > 7 || startRow < 0 || startRow  > 7) return false;
        if (targetCol < 0 || targetCol > 7 || targetRow < 0 || targetRow > 7) return false;

        int promoType = 0;
        if (text.length() >= 5) {
            switch (text[4]) {
            case 'q': promoType = W_QUEEN;  break;
            case 'r': promoType = W_ROOK;   break;
            case 'b': promoType = W_BISHOP; break;
            case 'n': promoType = W_KNIGHT; break;
            default: return false;
            }
        }

        // Legal hamle listesinde esini ara: terfi tasinin isareti buradan dogru
        // gelir ve gecersiz girdiler tahtayi bozamaz.
        std::vector<Move> legals = moveGen.getLegalMoves(board);
        for (size_t i = 0; i < legals.size(); i++) {
            const Move& m = legals[i];
            if (m.startRow != startRow || m.startCol != startCol) continue;
            if (m.targetRow != targetRow || m.targetCol != targetCol) continue;
            if (promoType == 0) { if (m.promotionPiece != 0) continue; }
            else { if (std::abs((int)m.promotionPiece) != promoType) continue; }
            out = m;
            return true;
        }
        return false;
    }

    // ============================================================
    //  SAN URETIMI
    // ============================================================
    static char pieceLetter(int type) {
        switch (type) {
        case W_KNIGHT: return 'N';
        case W_BISHOP: return 'B';
        case W_ROOK:   return 'R';
        case W_QUEEN:  return 'Q';
        case W_KING:   return 'K';
        default:       return ' ';
        }
    }

    std::string toSan(const Move& m, ChessBoard& board, MoveGenerator& moveGen) {
        if (m.isNull()) return "----";

        int piece = board.getPiece(m.startRow, m.startCol);
        if (piece == 0) return toUci(m);   // gecersiz hamle: UCI'ye dus

        int type = std::abs(piece);
        std::string san;

        // --- ROK ---
        if (type == W_KING && std::abs((int)m.targetCol - (int)m.startCol) == 2) {
            san = (m.targetCol == 6) ? "O-O" : "O-O-O";
        }
        else {
            bool isCapture = (board.getPiece(m.targetRow, m.targetCol) != 0);

            // En passant: hedef kare bos ama yine de bir alimdir.
            if (type == W_PAWN && m.startCol != m.targetCol && !isCapture) isCapture = true;

            if (type == W_PAWN) {
                // Piyon alimi kalkis dosyasiyla yazilir: exd5
                if (isCapture) { san += (char)('a' + m.startCol); san += 'x'; }
                san += (char)('a' + m.targetCol);
                san += std::to_string(8 - m.targetRow);
                if (m.promotionPiece != 0) {
                    san += '=';
                    san += pieceLetter(std::abs((int)m.promotionPiece));
                }
            }
            else {
                san += pieceLetter(type);

                // --- BELIRSIZLIK GIDERME ---
                // Ayni tur ve renkte baska bir tas da ayni kareye gidebiliyorsa
                // kalkis karesinden ayirt edici bilgi eklenir. Standart kural:
                //   1) dosyalar farkliysa dosya harfi yeter      -> Nbd2
                //   2) degilse satirlar farkliysa satir rakami   -> N1d2
                //   3) ikisi de ayniysa tam kare                 -> Nb1d2
                std::vector<Move> legals = moveGen.getLegalMoves(board);
                bool ambiguous = false, sameFile = false, sameRank = false;

                for (size_t i = 0; i < legals.size(); i++) {
                    const Move& o = legals[i];
                    if (o == m) continue;
                    if (o.targetRow != m.targetRow || o.targetCol != m.targetCol) continue;
                    int op = board.getPiece(o.startRow, o.startCol);
                    if (op != piece) continue;         // ayni tur VE ayni renk
                    ambiguous = true;
                    if (o.startCol == m.startCol) sameFile = true;
                    if (o.startRow == m.startRow) sameRank = true;
                }

                if (ambiguous) {
                    if (!sameFile) {
                        san += (char)('a' + m.startCol);
                    }
                    else if (!sameRank) {
                        san += std::to_string(8 - m.startRow);
                    }
                    else {
                        san += (char)('a' + m.startCol);
                        san += std::to_string(8 - m.startRow);
                    }
                }

                if (isCapture) san += 'x';
                san += (char)('a' + m.targetCol);
                san += std::to_string(8 - m.targetRow);
            }
        }

        // --- SAH / MAT ---
        // Hamleyi gecici olarak oynayip rakibin durumuna bakiyoruz.
        board.makeMove(m);
        bool givesCheck = board.isKingAttacked(board.currentPlayer);
        bool noReply = moveGen.getLegalMoves(board).empty();
        board.undoMove();

        if (givesCheck && noReply) san += '#';
        else if (givesCheck)       san += '+';

        return san;
    }

    std::vector<std::string> lineToSan(const std::vector<Move>& moves, ChessBoard& board, MoveGenerator& moveGen) {
        std::vector<std::string> out;
        size_t played = 0;
        for (size_t i = 0; i < moves.size(); i++) {
            out.push_back(toSan(moves[i], board, moveGen));
            board.makeMove(moves[i]);
            played++;
        }
        for (size_t i = 0; i < played; i++) board.undoMove();
        return out;
    }

}  // namespace Notation