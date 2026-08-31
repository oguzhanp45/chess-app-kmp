#include "EngineApi.hpp"
#include "Notation.hpp"
#include <algorithm>
#include <cstdlib>   // std::abs - libc++ bunu <algorithm> ile getirmiyor

EngineApi::EngineApi() {
    newGame();
}

// ---------------- OYUN KURULUMU ----------------
bool EngineApi::newGame(const std::string& fen) {
    std::string useFen = fen.empty() ? std::string(START_FEN) : fen;

    if (!board.setFen(useFen)) {
        board.initializeBoard();
        startFen = START_FEN;
        playedMoves.clear();
        gameHistory.clear();
        gameHistory.push_back(board.zobristKey);
        ai.clearTT();
        return false;
    }

    startFen = useFen;
    playedMoves.clear();
    gameHistory.clear();
    gameHistory.push_back(board.zobristKey);
    ai.clearTT();
    return true;
}

// ---------------- DURUM SORGULARI ----------------
std::string EngineApi::getFen() { return board.getFen(); }

std::string EngineApi::sideToMove() { return (board.currentPlayer == 1) ? "w" : "b"; }

bool EngineApi::inCheck() { return board.isKingAttacked(board.currentPlayer); }

std::vector<std::string> EngineApi::legalMoves() {
    std::vector<Move> moves = moveGen.getLegalMoves(board);
    std::vector<std::string> out;
    out.reserve(moves.size());
    for (size_t i = 0; i < moves.size(); i++) out.push_back(Notation::toUci(moves[i]));
    return out;
}

std::string EngineApi::gameStatus() {
    std::vector<Move> moves = moveGen.getLegalMoves(board);
    if (moves.empty()) {
        return inCheck() ? "checkmate" : "stalemate";
    }
    if (board.isInsufficientMaterial()) return "draw-material";
    if (board.halfMoveClock >= 100)      return "draw-fifty";
    if (board.repetitionCount() >= 3)    return "draw-repetition";
    return "ongoing";
}

std::vector<std::string> EngineApi::moveHistorySan() {
    // Baslangic pozisyonuna donup hamleleri tekrar oynayarak SAN uretiyoruz.
    // SAN, oynandigi ANDAKI pozisyona bagli oldugu icin baska yolu yok.
    ChessBoard tmp;
    tmp.setFen(startFen);
    MoveGenerator mg;
    std::vector<std::string> out;
    out.reserve(playedMoves.size());
    for (size_t i = 0; i < playedMoves.size(); i++) {
        out.push_back(Notation::toSan(playedMoves[i], tmp, mg));
        tmp.makeMove(playedMoves[i]);
    }
    return out;
}

// ---------------- HAMLE ----------------
bool EngineApi::findLegal(const std::string& uci, Move& out) {
    return Notation::fromUci(uci, board, moveGen, out);
}

bool EngineApi::makeMove(const std::string& uci) {
    Move m;
    if (!findLegal(uci, m)) return false;
    board.makeMove(m);
    playedMoves.push_back(m);
    gameHistory.push_back(board.zobristKey);
    return true;
}

bool EngineApi::undo() {
    if (playedMoves.empty()) return false;
    board.undoMove();
    playedMoves.pop_back();
    if (!gameHistory.empty()) gameHistory.pop_back();
    return true;
}

std::string EngineApi::sanFor(const std::string& uci) {
    Move m;
    if (!findLegal(uci, m)) return "";
    return Notation::toSan(m, board, moveGen);
}

// ---------------- ARAMA ----------------
std::string EngineApi::bestMove(int timeMs, int maxDepth) {
    if (timeMs < 10) timeMs = 10;
    Move m = ai.getBestMoveTimed(board, maxDepth, timeMs, timeMs, gameHistory);
    if (m.isNull()) return "";     // mat veya pat
    return Notation::toUci(m);
}

std::vector<ScoredMove> EngineApi::bestMoves(int n, int timeMs, int maxDepth) {
    std::vector<ScoredMove> out;
    if (n <= 0) return out;
    if (timeMs < 10) timeMs = 10;

    // Analiz her zaman tam guc ve kitapsiz. Cagiranin ayarlarini
    // bozmuyoruz: gecici olarak degistirip geri koyuyoruz.
    int  savedSkill = ai.getSkillLevel();
    bool savedBook = ai.useBook;

    ai.setSkillLevel(20);
    ai.useBook = false;
    ai.wideRoot = true;

    ai.getBestMoveTimed(board, maxDepth, timeMs, timeMs, gameHistory);

    ai.wideRoot = false;
    ai.useBook = savedBook;
    ai.setSkillLevel(savedSkill);

    std::vector<RootLine> lines = ai.getRootLines();
    std::stable_sort(lines.begin(), lines.end(),
        [](const RootLine& a, const RootLine& b) { return a.score > b.score; });

    for (size_t i = 0; i < lines.size() && (int)out.size() < n; i++) {
        ScoredMove sm;
        sm.uci = Notation::toUci(lines[i].move);

        // Mat skorlari MATE_VALUE - ply olarak kodlu; disariya hamle
        // sayisi olarak veriyoruz (UCI "score mate N" ile ayni kural).
        int sc = lines[i].score;
        if (std::abs(sc) > MATE_THRESHOLD) {
            int matePlies = MATE_VALUE - std::abs(sc);
            int mateMoves = (matePlies + 1) / 2;
            sm.mateIn = (sc < 0) ? -mateMoves : mateMoves;
            sm.scoreCp = 0;
        }
        else {
            sm.mateIn = 0;
            sm.scoreCp = sc;
        }

        for (size_t j = 0; j < lines[i].pv.size(); j++)
            sm.pv.push_back(Notation::toUci(lines[i].pv[j]));

        out.push_back(sm);
    }
    return out;
}

Evaluation EngineApi::evaluate() {
    Evaluation ev;

    // Bitmis pozisyonda degerlendirilecek bir sey yok. Sessizlik aramasi
    // yalnizca alimlara bakar; mati veya pati gormez, o yuzden pat
    // pozisyonunda alakasiz bir sayi dondururdu. Cagiran zaten
    // gameStatus()'e bakmak zorunda: sonucu o soyler.
    if (gameStatus() != "ongoing") return ev;   // {0, 0}

    int sc = ai.evaluateQuiet(board);

    if (std::abs(sc) > MATE_THRESHOLD) {
        int matePlies = MATE_VALUE - std::abs(sc);
        int mateMoves = (matePlies + 1) / 2;
        ev.mateIn = (sc < 0) ? -mateMoves : mateMoves;
        ev.scoreCp = 0;
    }
    else {
        ev.mateIn = 0;
        ev.scoreCp = sc;
    }
    return ev;
}

void EngineApi::stop() { ai.stop(); }

// ---------------- AYARLAR ----------------
void EngineApi::setSkillLevel(int level) { ai.setSkillLevel(level); }
int  EngineApi::getSkillLevel() const { return ai.getSkillLevel(); }
void EngineApi::setHashSizeMB(int mb) { ai.setHashSizeMB(mb); }
void EngineApi::setUseBook(bool on) { ai.useBook = on; }

bool EngineApi::loadBookFromFile(const std::string& path) {
    ai.bookPath = path;
    ai.unloadBook();          // yol degisti, eski kitap gecersiz
    return ai.ensureBookLoaded();
}

bool EngineApi::isBookLoaded() const { return ai.isBookLoaded(); }

std::vector<BookMove> EngineApi::bookMoves() {
    std::vector<BookMove> out;
    ai.ensureBookLoaded();

    std::vector<BookEntry> entries = ai.getBookEntries(board);
    if (entries.empty()) return out;

    unsigned long long total = 0;
    for (size_t i = 0; i < entries.size(); i++) total += entries[i].weight;

    for (size_t i = 0; i < entries.size(); i++) {
        BookMove bm;
        bm.uci = Notation::toUci(entries[i].move);
        bm.weight = (int)entries[i].weight;
        // Yuvarlama: agirliksiz kitapta (total 0) yuzde de 0 kalir.
        bm.percent = (total > 0)
            ? (int)((entries[i].weight * 200ULL + total) / (total * 2))
            : 0;
        out.push_back(bm);
    }
    return out;
}

bool EngineApi::loadBookFromMemory(const unsigned char* bytes, size_t size) {
    return ai.loadBookFromMemory(bytes, size);
}

void EngineApi::setInfoCallback(std::function<void(const std::string&)> cb) {
    ai.setInfoCallback(cb);
}