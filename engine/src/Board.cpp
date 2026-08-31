#include "Board.hpp"
#include "PolyglotKeys.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cmath>

// ============================================================
//  ZOBRIST INDEKS HESAPLAYICI  (Polyglot standardina gore)
// ============================================================
// Polyglot sirasi: 0=siyah piyon, 1=beyaz piyon, 2=siyah at, 3=beyaz at, ...
// yani  indeks = (tas_tipi - 1) * 2 + (beyaz ? 1 : 0)
//
// Onceki surumde burada beyaz=0 / siyah=1 kullaniliyordu. Kendi icinde
// tutarliydi ama Polyglot standardiyla uyusmuyordu; bu yuzden motor iki
// ayri hash tasimak zorunda kaliyordu:
//   - board.zobristKey  (dahili, hicbir yerde kullanilmiyordu)
//   - Polyglot::computeHash  (her dugumde 64 kareyi bastan tarayan)
// Konvansiyon duzeltilince ikisi ayni sey oldu; artik tek ve artimli
// olan kullaniliyor.
static inline int getZobristIndex(int piece, int r, int c) {
    bool isWhite = piece > 0;
    int type = std::abs(piece);
    int pIdx = type - 1;
    int polyPiece = (pIdx * 2) + (isWhite ? 1 : 0);
    int polySquare = (7 - r) * 8 + c;
    return 64 * polyPiece + polySquare;
}

// DURUM HASH'INI HESAPLA (Taslar haric)
void ChessBoard::generateHash() {
    zobristKey = pieceHash;
    if (whiteCastleK) zobristKey ^= Random64[768];
    if (whiteCastleQ) zobristKey ^= Random64[769];
    if (blackCastleK) zobristKey ^= Random64[770];
    if (blackCastleQ) zobristKey ^= Random64[771];

    if (enPassantCol != -1) {
        bool canCapture = false;
        int epRow = (currentPlayer == 1) ? 3 : 4;
        int myPawn = (currentPlayer == 1) ? W_PAWN : B_PAWN;
        if (enPassantCol > 0 && board[epRow][enPassantCol - 1] == myPawn) canCapture = true;
        if (enPassantCol < 7 && board[epRow][enPassantCol + 1] == myPawn) canCapture = true;
        if (canCapture) zobristKey ^= Random64[772 + enPassantCol];
    }
    if (currentPlayer == 1) zobristKey ^= Random64[780];
}

// --- SAH KARELERI ---
void ChessBoard::refreshKingSquares() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == W_KING) { whiteKingRow = r; whiteKingCol = c; }
            else if (board[r][c] == B_KING) { blackKingRow = r; blackKingCol = c; }
        }
    }
}

bool ChessBoard::isKingAttacked(int color) {
    if (color == 1) return isSquareAttacked(whiteKingRow, whiteKingCol, -1);
    return isSquareAttacked(blackKingRow, blackKingCol, 1);
}

// Takip edilen kare gercekten sahin oldugu kare mi? (sadece testte kullanilir)
bool ChessBoard::debugKingSquaresValid() {
    int wr = -1, wc = -1, br = -1, bc = -1;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == W_KING) { wr = r; wc = c; }
            else if (board[r][c] == B_KING) { br = r; bc = c; }
        }
    }
    if (wr != -1 && (wr != whiteKingRow || wc != whiteKingCol)) return false;
    if (br != -1 && (br != blackKingRow || bc != blackKingCol)) return false;
    return true;
}

// --- Tahtadan sifirdan tas hash'i hesapla (dogrulama amacli) ---
unsigned long long ChessBoard::debugRecomputePieceHash() {
    unsigned long long h = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != EMPTY) h ^= Random64[getZobristIndex(board[i][j], i, j)];
        }
    }
    return h;
}

void ChessBoard::initializeBoard() {
    currentPlayer = 1;

    whiteCastleK = true; whiteCastleQ = true;
    blackCastleK = true; blackCastleQ = true;

    enPassantRow = -1;
    enPassantCol = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    history.clear();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = EMPTY;
        }
    }

    board[0][0] = B_ROOK;   board[0][1] = B_KNIGHT; board[0][2] = B_BISHOP;
    board[0][3] = B_QUEEN;  board[0][4] = B_KING;   board[0][5] = B_BISHOP;
    board[0][6] = B_KNIGHT; board[0][7] = B_ROOK;
    for (int j = 0; j < 8; j++) board[1][j] = B_PAWN;

    for (int j = 0; j < 8; j++) board[6][j] = W_PAWN;
    board[7][0] = W_ROOK;   board[7][1] = W_KNIGHT; board[7][2] = W_BISHOP;
    board[7][3] = W_QUEEN;  board[7][4] = W_KING;   board[7][5] = W_BISHOP;
    board[7][6] = W_KNIGHT; board[7][7] = W_ROOK;

    refreshKingSquares();
    pieceHash = debugRecomputePieceHash();
    generateHash();
}

// --- FEN OKUMA ---
// Basarisiz olursa false doner ve mevcut tahtaya dokunmaz.
bool ChessBoard::setFen(const std::string& fen) {
    int tmp[8][8] = { {0} };

    std::istringstream ss(fen);
    std::string boardPart, sidePart, castlePart, epPart;

    if (!(ss >> boardPart)) return false;
    if (!(ss >> sidePart))   sidePart = "w";
    if (!(ss >> castlePart)) castlePart = "-";
    if (!(ss >> epPart))     epPart = "-";

    long long half = 0, full = 1;
    if (!(ss >> half)) { half = 0; ss.clear(); }
    if (!(ss >> full)) { full = 1; }

    int r = 0, c = 0;
    for (size_t i = 0; i < boardPart.size(); i++) {
        char ch = boardPart[i];
        if (ch == '/') {
            if (c != 8) return false;
            r++; c = 0;
            if (r > 7) return false;
            continue;
        }
        if (ch >= '1' && ch <= '8') {
            c += (ch - '0');
            if (c > 8) return false;
            continue;
        }
        if (r > 7 || c > 7) return false;

        int piece = 0;
        switch (std::tolower((unsigned char)ch)) {
        case 'p': piece = W_PAWN;   break;
        case 'n': piece = W_KNIGHT; break;
        case 'b': piece = W_BISHOP; break;
        case 'r': piece = W_ROOK;   break;
        case 'q': piece = W_QUEEN;  break;
        case 'k': piece = W_KING;   break;
        default: return false;
        }
        if (std::islower((unsigned char)ch)) piece = -piece;
        tmp[r][c] = piece;
        c++;
    }
    if (r != 7 || c != 8) return false;

    // Parse basarili -> simdi tahtayi yaz
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            board[i][j] = tmp[i][j];

    currentPlayer = (sidePart == "b") ? -1 : 1;

    whiteCastleK = (castlePart.find('K') != std::string::npos);
    whiteCastleQ = (castlePart.find('Q') != std::string::npos);
    blackCastleK = (castlePart.find('k') != std::string::npos);
    blackCastleQ = (castlePart.find('q') != std::string::npos);

    // Rok haklarini gercek tas yerlesimiyle dogrula.
    // (Bozuk bir FEN, olmayan bir kaleyi oynatmaya calismasin diye.)
    if (board[7][4] != W_KING) { whiteCastleK = false; whiteCastleQ = false; }
    if (board[7][7] != W_ROOK) whiteCastleK = false;
    if (board[7][0] != W_ROOK) whiteCastleQ = false;
    if (board[0][4] != B_KING) { blackCastleK = false; blackCastleQ = false; }
    if (board[0][7] != B_ROOK) blackCastleK = false;
    if (board[0][0] != B_ROOK) blackCastleQ = false;

    enPassantRow = -1;
    enPassantCol = -1;
    if (epPart != "-" && epPart.size() >= 2) {
        int f = epPart[0] - 'a';
        int rank = epPart[1] - '0';
        if (f >= 0 && f < 8 && rank >= 1 && rank <= 8) {
            enPassantCol = f;
            enPassantRow = 8 - rank;
        }
    }

    halfMoveClock = (int)half;
    fullMoveNumber = (full > 0) ? (int)full : 1;

    // ============================================================
    //  FEN GECERLILIK DENETIMI
    // ============================================================
    // Onceki surum sahsiz ya da kural disi bir pozisyonu sessizce kabul
    // ediyordu. Iki somut sonucu vardi:
    //   1) Sah yoksa isKingAttacked takip edilmeyen bir kareye bakar;
    //      motor ya sacmalar ya da coker.
    //   2) Sira bizdeyken RAKIBIN sahi tehdit altindaysa pozisyon zaten
    //      kural disidir; hamle ureteci "sah alma" hamlesi uretir ve
    //      takip edilen sah karesi tutarsiz hale gelir.
    // Acik bir depoda ilk denenecek seylerden biri budur.
    int wKings = 0, bKings = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == W_KING) wKings++;
            else if (board[i][j] == B_KING) bKings++;
        }
    }
    if (wKings != 1 || bKings != 1) {
        // Tahtayi bozmus olduk; cagiran tarafin toparlamasi icin false.
        return false;
    }

    refreshKingSquares();

    // Sira bizdeyse rakibin sahi tehdit altinda OLAMAZ.
    if (isKingAttacked(-currentPlayer)) return false;

    history.clear();
    pieceHash = debugRecomputePieceHash();
    generateHash();
    return true;
}

// --- FEN YAZMA ---
std::string ChessBoard::getFen() {
    std::string fen;

    for (int r = 0; r < 8; r++) {
        int emptyCount = 0;
        for (int c = 0; c < 8; c++) {
            int p = board[r][c];
            if (p == EMPTY) { emptyCount++; continue; }
            if (emptyCount > 0) { fen += std::to_string(emptyCount); emptyCount = 0; }

            char ch = '?';
            switch (std::abs(p)) {
            case W_PAWN:   ch = 'p'; break;
            case W_KNIGHT: ch = 'n'; break;
            case W_BISHOP: ch = 'b'; break;
            case W_ROOK:   ch = 'r'; break;
            case W_QUEEN:  ch = 'q'; break;
            case W_KING:   ch = 'k'; break;
            }
            if (p > 0) ch = (char)std::toupper((unsigned char)ch);
            fen += ch;
        }
        if (emptyCount > 0) fen += std::to_string(emptyCount);
        if (r < 7) fen += '/';
    }

    fen += ' ';
    fen += (currentPlayer == 1) ? 'w' : 'b';

    fen += ' ';
    std::string castle;
    if (whiteCastleK) castle += 'K';
    if (whiteCastleQ) castle += 'Q';
    if (blackCastleK) castle += 'k';
    if (blackCastleQ) castle += 'q';
    fen += castle.empty() ? "-" : castle;

    fen += ' ';
    if (enPassantRow >= 0 && enPassantCol >= 0) {
        fen += (char)('a' + enPassantCol);
        fen += std::to_string(8 - enPassantRow);
    }
    else {
        fen += '-';
    }

    fen += ' ' + std::to_string(halfMoveClock);
    fen += ' ' + std::to_string(fullMoveNumber);
    return fen;
}

void ChessBoard::printBoard() {
    std::cout << "\n  a b c d e f g h\n";
    for (int i = 0; i < 8; i++) {
        std::cout << 8 - i << " ";
        for (int j = 0; j < 8; j++) {
            int piece = board[i][j];
            switch (piece) {
            case B_ROOK:   std::cout << "k "; break;
            case B_KNIGHT: std::cout << "a "; break;
            case B_BISHOP: std::cout << "f "; break;
            case B_QUEEN:  std::cout << "v "; break;
            case B_KING:   std::cout << "s "; break;
            case B_PAWN:   std::cout << "p "; break;

            case W_ROOK:   std::cout << "K "; break;
            case W_KNIGHT: std::cout << "A "; break;
            case W_BISHOP: std::cout << "F "; break;
            case W_QUEEN:  std::cout << "V "; break;
            case W_KING:   std::cout << "S "; break;
            case W_PAWN:   std::cout << "P "; break;

            case EMPTY:    std::cout << ". "; break;
            }
        }
        std::cout << 8 - i << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

bool ChessBoard::isWhite(int x, int y) {
    return board[x][y] > 0;
}

int ChessBoard::getPiece(int row, int col) {
    return board[row][col];
}

void ChessBoard::makeMove(Move move) {
    int movingPiece = board[move.startRow][move.startCol];
    int targetPiece = board[move.targetRow][move.targetCol];

    MoveRecord record;
    record.move = move;
    record.whiteCastleK = whiteCastleK;
    record.whiteCastleQ = whiteCastleQ;
    record.blackCastleK = blackCastleK;
    record.blackCastleQ = blackCastleQ;
    record.enPassantRow = enPassantRow;
    record.enPassantCol = enPassantCol;
    record.halfMoveClock = halfMoveClock;

    record.zobristKey = zobristKey;
    record.pieceHash = pieceHash;

    if (abs(movingPiece) == 1 || targetPiece != 0) {
        halfMoveClock = 0;
    }
    else {
        halfMoveClock++;
    }

    bool isEnPassant = (abs(movingPiece) == 1 && move.targetRow == enPassantRow && move.targetCol == enPassantCol);
    if (isEnPassant) {
        record.capturedPiece = board[move.startRow][move.targetCol];
        board[move.startRow][move.targetCol] = EMPTY;
        pieceHash ^= Random64[getZobristIndex(record.capturedPiece, move.startRow, move.targetCol)];
    }
    else {
        record.capturedPiece = targetPiece;
        if (targetPiece != EMPTY) {
            pieceHash ^= Random64[getZobristIndex(targetPiece, move.targetRow, move.targetCol)];
        }
    }

    history.push_back(record);

    pieceHash ^= Random64[getZobristIndex(movingPiece, move.startRow, move.startCol)];

    if (move.promotionPiece != 0) {
        board[move.targetRow][move.targetCol] = move.promotionPiece;
        pieceHash ^= Random64[getZobristIndex(move.promotionPiece, move.targetRow, move.targetCol)];
    }
    else {
        board[move.targetRow][move.targetCol] = movingPiece;
        pieceHash ^= Random64[getZobristIndex(movingPiece, move.targetRow, move.targetCol)];
    }
    board[move.startRow][move.startCol] = EMPTY;

    // sah oynadiysa takip edilen kareyi guncelle
    if (movingPiece == W_KING) { whiteKingRow = move.targetRow; whiteKingCol = move.targetCol; }
    else if (movingPiece == B_KING) { blackKingRow = move.targetRow; blackKingCol = move.targetCol; }

    if (abs(movingPiece) == W_KING && abs(move.targetCol - move.startCol) == 2) {
        if (move.targetCol == 6) {
            int rook = board[move.startRow][7];
            board[move.startRow][5] = rook;
            board[move.startRow][7] = EMPTY;
            pieceHash ^= Random64[getZobristIndex(rook, move.startRow, 7)];
            pieceHash ^= Random64[getZobristIndex(rook, move.startRow, 5)];
        }
        else if (move.targetCol == 2) {
            int rook = board[move.startRow][0];
            board[move.startRow][3] = rook;
            board[move.startRow][0] = EMPTY;
            pieceHash ^= Random64[getZobristIndex(rook, move.startRow, 0)];
            pieceHash ^= Random64[getZobristIndex(rook, move.startRow, 3)];
        }
    }

    if (abs(movingPiece) == 1 && abs(move.targetRow - move.startRow) == 2) {
        enPassantRow = (move.startRow + move.targetRow) / 2;
        enPassantCol = move.startCol;
    }
    else {
        enPassantRow = -1;
        enPassantCol = -1;
    }

    if (movingPiece == W_KING) { whiteCastleK = false; whiteCastleQ = false; }
    if (movingPiece == B_KING) { blackCastleK = false; blackCastleQ = false; }
    if (movingPiece == W_ROOK) {
        if (move.startRow == 7 && move.startCol == 0) whiteCastleQ = false;
        if (move.startRow == 7 && move.startCol == 7) whiteCastleK = false;
    }
    if (movingPiece == B_ROOK) {
        if (move.startRow == 0 && move.startCol == 0) blackCastleQ = false;
        if (move.startRow == 0 && move.startCol == 7) blackCastleK = false;
    }
    if (targetPiece == W_ROOK) {
        if (move.targetRow == 7 && move.targetCol == 0) whiteCastleQ = false;
        if (move.targetRow == 7 && move.targetCol == 7) whiteCastleK = false;
    }
    if (targetPiece == B_ROOK) {
        if (move.targetRow == 0 && move.targetCol == 0) blackCastleQ = false;
        if (move.targetRow == 0 && move.targetCol == 7) blackCastleK = false;
    }

    if (currentPlayer == -1) fullMoveNumber++;
    currentPlayer *= -1;

    generateHash();
}

void ChessBoard::undoMove() {
    if (history.empty()) return;

    MoveRecord record = history.back();
    history.pop_back();

    Move m = record.move;
    int movingPiece = board[m.targetRow][m.targetCol];

    if (m.promotionPiece != 0) {
        movingPiece = (currentPlayer == -1) ? W_PAWN : B_PAWN;
    }

    board[m.startRow][m.startCol] = movingPiece;

    // sah geri alindiysa kareyi de geri al
    if (movingPiece == W_KING) { whiteKingRow = m.startRow; whiteKingCol = m.startCol; }
    else if (movingPiece == B_KING) { blackKingRow = m.startRow; blackKingCol = m.startCol; }

    bool wasEnPassant = (abs(movingPiece) == 1 && m.targetRow == record.enPassantRow && m.targetCol == record.enPassantCol);
    if (wasEnPassant) {
        board[m.targetRow][m.targetCol] = EMPTY;
        board[m.startRow][m.targetCol] = record.capturedPiece;
    }
    else {
        board[m.targetRow][m.targetCol] = record.capturedPiece;
    }

    if (abs(movingPiece) == W_KING && abs(m.targetCol - m.startCol) == 2) {
        if (m.targetCol == 6) {
            board[m.startRow][7] = board[m.startRow][5];
            board[m.startRow][5] = EMPTY;
        }
        else if (m.targetCol == 2) {
            board[m.startRow][0] = board[m.startRow][3];
            board[m.startRow][3] = EMPTY;
        }
    }

    whiteCastleK = record.whiteCastleK;
    whiteCastleQ = record.whiteCastleQ;
    blackCastleK = record.blackCastleK;
    blackCastleQ = record.blackCastleQ;
    enPassantRow = record.enPassantRow;
    enPassantCol = record.enPassantCol;

    halfMoveClock = record.halfMoveClock;
    currentPlayer *= -1;
    if (currentPlayer == -1) fullMoveNumber--;

    zobristKey = record.zobristKey;
    pieceHash = record.pieceHash;
}

bool ChessBoard::isSquareAttacked(int row, int col, int attackerColor) {
    if (row < 0 || row > 7 || col < 0 || col > 7) return false;

    int pawnRowDir = (attackerColor == 1) ? 1 : -1;
    int pRow = row + pawnRowDir;

    if (pRow >= 0 && pRow < 8) {
        if (col - 1 >= 0 && board[pRow][col - 1] == attackerColor * W_PAWN) return true;
        if (col + 1 < 8 && board[pRow][col + 1] == attackerColor * W_PAWN) return true;
    }

    int knightRow[] = { -2, -2, -1, -1,  1,  1,  2,  2 };
    int knightCol[] = { -1,  1, -2,  2, -2,  2, -1,  1 };
    for (int i = 0; i < 8; i++) {
        int r = row + knightRow[i];
        int c = col + knightCol[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            if (board[r][c] == attackerColor * W_KNIGHT) return true;
        }
    }

    int straightRow[] = { -1, 1, 0, 0 };
    int straightCol[] = { 0, 0, -1, 1 };
    for (int i = 0; i < 4; i++) {
        int r = row + straightRow[i];
        int c = col + straightCol[i];
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            int piece = board[r][c];
            if (piece != EMPTY) {
                if (piece == attackerColor * W_ROOK || piece == attackerColor * W_QUEEN) return true;
                break;
            }
            r += straightRow[i];
            c += straightCol[i];
        }
    }

    int diagRow[] = { -1, -1, 1, 1 };
    int diagCol[] = { -1, 1, -1, 1 };
    for (int i = 0; i < 4; i++) {
        int r = row + diagRow[i];
        int c = col + diagCol[i];
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            int piece = board[r][c];
            if (piece != EMPTY) {
                if (piece == attackerColor * W_BISHOP || piece == attackerColor * W_QUEEN) return true;
                break;
            }
            r += diagRow[i];
            c += diagCol[i];
        }
    }

    int kingRow[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    int kingCol[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    for (int i = 0; i < 8; i++) {
        int r = row + kingRow[i];
        int c = col + kingCol[i];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            if (board[r][c] == attackerColor * W_KING) return true;
        }
    }

    return false;
}

// ============================================================
//  BERABERLIK KURALLARI
// ============================================================
//
// Onceki surumde tekrar sayaci bir unordered_map ile tutuluyordu ve
// iki sorunu vardi:
//   1) makeMove basta mevcut anahtari azaltiyor, undoMove geri koymuyordu.
//      Bir make/undo cifti sonrasi sayac 1'den 0'a dusuyor ve bir daha
//      geri gelmiyordu -> uc tekrar hicbir zaman tespit edilemiyordu.
//   2) Arama sirasinda ziyaret edilen HER benzersiz pozisyon icin haritada
//      kalici bir kayit aciliyordu. Milyonlarca dugumluk bir aramada bu,
//      yuzlerce MB'lik kontrolsuz bellek buyumesi demekti.
//
// Cozum: harita tamamen kaldirildi. Tekrar sayimi zaten elimizde olan
// history vektorunden yapiliyor; geri donusu olmayan son hamleden oteye
// bakmaya gerek olmadigi icin tarama halfMoveClock ile sinirli.

int ChessBoard::repetitionCount() {
    int count = 1;
    int n = (int)history.size();
    int limit = (halfMoveClock < n) ? halfMoveClock : n;

    // Sadece ayni tarafin oynayacagi pozisyonlar tekrar olabilir -> 2 adim atla.
    for (int i = 1; i <= limit; i += 2) {
        int idx = n - 1 - i;
        if (idx < 0) break;
        if (history[idx].zobristKey == zobristKey) count++;
    }
    return count;
}

bool ChessBoard::isInsufficientMaterial() {
    int wKnight = 0, wBishop = 0, bKnight = 0, bBishop = 0;
    int wBishopSq = -1, bBishopSq = -1;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int p = board[r][c];
            if (p == EMPTY) continue;
            int t = std::abs(p);

            // Piyon, kale veya vezir varsa mat hala mumkundur.
            if (t == W_PAWN || t == W_ROOK || t == W_QUEEN) return false;

            if (t == W_KNIGHT) { if (p > 0) wKnight++; else bKnight++; }
            else if (t == W_BISHOP) {
                if (p > 0) { wBishop++; wBishopSq = (r + c) & 1; }
                else { bBishop++; bBishopSq = (r + c) & 1; }
            }
        }
    }

    int wMinor = wKnight + wBishop;
    int bMinor = bKnight + bBishop;

    if (wMinor == 0 && bMinor == 0) return true;              // K vs K
    if (wMinor == 1 && bMinor == 0) return true;              // K+hafif vs K
    if (bMinor == 1 && wMinor == 0) return true;              // K vs K+hafif

    // K+F vs K+F, filler ayni renk karede
    if (wKnight == 0 && bKnight == 0 && wBishop == 1 && bBishop == 1 && wBishopSq == bBishopSq)
        return true;

    // K+A+A vs K teknik olarak beraberedir ama zorlanamaz; beraberlik saymiyoruz.
    return false;
}

bool ChessBoard::isDraw() {
    return (halfMoveClock >= 100) || (repetitionCount() >= 3) || isInsufficientMaterial();
}