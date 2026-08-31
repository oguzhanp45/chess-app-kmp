#include "chess_c_api.h"
#include "EngineApi.hpp"

#include <atomic>
#include <sstream>
#include <string>
#include <vector>

// ============================================================
//  Kucuk JSON yazici
// ============================================================
// Bagimlilik eklemiyoruz. Sinirdan gecen her dize UCI, SAN, FEN ya da
// durum adi; hicbirinde tirnak ya da ters bolu yok. Yine de kacis
// fonksiyonu duruyor -- ileride beklenmedik bir dize gecerse cikti
// bozulmasin diye.

namespace {

const char* const kVersion = "cpp-chess-engine 1.0 (C++17)";

void appendEscaped(std::string& out, const std::string& value) {
    for (char c : value) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Kontrol karakteri: \u00XX. Buraya hic gelinmemeli.
                    static const char* hex = "0123456789abcdef";
                    out += "\\u00";
                    out += hex[(c >> 4) & 0x0F];
                    out += hex[c & 0x0F];
                } else {
                    out += c;
                }
        }
    }
}

void appendString(std::string& out, const std::string& value) {
    out += '"';
    appendEscaped(out, value);
    out += '"';
}

void appendInt(std::string& out, int value) {
    out += std::to_string(value);
}

void appendStringArray(std::string& out, const std::vector<std::string>& values) {
    out += '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) out += ',';
        appendString(out, values[i]);
    }
    out += ']';
}

// UCI info satirini ayristirir:
//   "info depth 8 seldepth 12 score cp 34 nodes 1234 time 56 nps ... pv e2e4 ..."
// pv'ye gelince duruyoruz; gerisi hamle listesi ve bize burada gerekmiyor.
struct InfoLine {
    int depth = 0;
    int selDepth = 0;
    int scoreCp = 0;
    int mateIn = 0;
    long long nodes = 0;
    int timeMs = 0;
};

InfoLine parseInfoLine(const std::string& line) {
    InfoLine out;
    std::istringstream in(line);
    std::string token;
    while (in >> token) {
        if (token == "depth") {
            in >> out.depth;
        } else if (token == "seldepth") {
            in >> out.selDepth;
        } else if (token == "nodes") {
            in >> out.nodes;
        } else if (token == "time") {
            in >> out.timeMs;
        } else if (token == "score") {
            std::string kind;
            in >> kind;
            if (kind == "cp") {
                in >> out.scoreCp;
                out.mateIn = 0;
            } else if (kind == "mate") {
                in >> out.mateIn;
                out.scoreCp = 0;
            }
        } else if (token == "pv") {
            break;
        }
    }
    return out;
}

} // namespace

// ============================================================
//  Tutamac
// ============================================================
// Her dize donduren fonksiyonun KENDI tamponu var. Boylece
// chess_san_for cagrisi, elde tutulan bir snapshot isaretcisini
// gecersiz kilmiyor.

struct chess_engine {
    EngineApi api;

    // Her dize donduren fonksiyonun kendi tamponu var.
    std::string snapshotBuf;
    std::string sanBuf;
    std::string bestMoveBuf;
    std::string bestMovesBuf;
    std::string evalBuf;
    std::string bookBuf;

    // Canli arama bilgisi. Arama IS PARCACIGINDAN yazilir, arayuz
    // parcacigindan okunur -- bu yuzden atomik. Motorun kendisine
    // dokunmadiklari icin arama surerken okunmalari guvenli.
    std::atomic<int>       infoDepth{0};
    std::atomic<int>       infoSelDepth{0};
    std::atomic<int>       infoScoreCp{0};
    std::atomic<int>       infoMateIn{0};
    std::atomic<long long> infoNodes{0};
    std::atomic<int>       infoTimeMs{0};

    chess_engine() {
        // Motor stdout'a yazmiyor; her "info ..." satiri buraya akiyor.
        // Mobilde stdout okuyan kimse yok, bu sart.
        api.setInfoCallback([this](const std::string& line) {
            const InfoLine parsed = parseInfoLine(line);
            infoDepth.store(parsed.depth, std::memory_order_relaxed);
            infoSelDepth.store(parsed.selDepth, std::memory_order_relaxed);
            infoScoreCp.store(parsed.scoreCp, std::memory_order_relaxed);
            infoMateIn.store(parsed.mateIn, std::memory_order_relaxed);
            infoNodes.store(parsed.nodes, std::memory_order_relaxed);
            infoTimeMs.store(parsed.timeMs, std::memory_order_relaxed);
        });
    }

    void resetInfo() {
        infoDepth.store(0, std::memory_order_relaxed);
        infoSelDepth.store(0, std::memory_order_relaxed);
        infoScoreCp.store(0, std::memory_order_relaxed);
        infoMateIn.store(0, std::memory_order_relaxed);
        infoNodes.store(0, std::memory_order_relaxed);
        infoTimeMs.store(0, std::memory_order_relaxed);
    }
};

// ------------------------------------------------------------
//  Surum ve oz test
// ------------------------------------------------------------

extern "C" const char* chess_version(void) {
    return kVersion;
}

extern "C" int chess_selftest(void) {
    try {
        EngineApi engine;
        return static_cast<int>(engine.legalMoves().size());
    } catch (...) {
        return -1;
    }
}

// ------------------------------------------------------------
//  Tutamac yonetimi
// ------------------------------------------------------------

extern "C" chess_engine* chess_create(void) {
    try {
        return new chess_engine();
    } catch (...) {
        return nullptr;
    }
}

extern "C" void chess_destroy(chess_engine* engine) {
    delete engine;
}

// ------------------------------------------------------------
//  Oyun kurulumu ve durum
// ------------------------------------------------------------

extern "C" int chess_new_game(chess_engine* engine, const char* fen) {
    if (engine == nullptr) return 0;
    try {
        return engine->api.newGame(fen != nullptr ? std::string(fen) : std::string()) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" const char* chess_snapshot_json(chess_engine* engine) {
    if (engine == nullptr) return "{}";
    try {
        std::string& out = engine->snapshotBuf;
        out.clear();
        out.reserve(1024);

        out += "{\"fen\":";
        appendString(out, engine->api.getFen());

        out += ",\"side\":";
        appendString(out, engine->api.sideToMove());

        out += ",\"inCheck\":";
        out += engine->api.inCheck() ? "true" : "false";

        out += ",\"status\":";
        appendString(out, engine->api.gameStatus());

        out += ",\"legal\":";
        appendStringArray(out, engine->api.legalMoves());

        out += ",\"history\":";
        appendStringArray(out, engine->api.moveHistorySan());

        out += '}';
        return out.c_str();
    } catch (...) {
        return "{}";
    }
}

// ------------------------------------------------------------
//  Hamle
// ------------------------------------------------------------

extern "C" int chess_make_move(chess_engine* engine, const char* uci) {
    if (engine == nullptr || uci == nullptr) return 0;
    try {
        return engine->api.makeMove(std::string(uci)) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" int chess_undo(chess_engine* engine) {
    if (engine == nullptr) return 0;
    try {
        return engine->api.undo() ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" const char* chess_san_for(chess_engine* engine, const char* uci) {
    if (engine == nullptr || uci == nullptr) return "";
    try {
        engine->sanBuf = engine->api.sanFor(std::string(uci));
        return engine->sanBuf.c_str();
    } catch (...) {
        return "";
    }
}

// ------------------------------------------------------------
//  Arama -- BLOKE EDER
// ------------------------------------------------------------

namespace {

// maxDepth 0 verilirse pratikte sinirsiz.
inline int depthOrDefault(int maxDepth) {
    return maxDepth > 0 ? maxDepth : 64;
}

} // namespace

extern "C" const char* chess_best_move(chess_engine* engine, int timeMs, int maxDepth) {
    if (engine == nullptr) return "";
    try {
        engine->resetInfo();
        engine->bestMoveBuf = engine->api.bestMove(timeMs, depthOrDefault(maxDepth));
        return engine->bestMoveBuf.c_str();
    } catch (...) {
        return "";
    }
}

extern "C" const char* chess_best_moves_json(chess_engine* engine, int n,
                                             int timeMs, int maxDepth) {
    if (engine == nullptr) return "{\"moves\":[]}";
    try {
        engine->resetInfo();
        const std::vector<ScoredMove> moves =
            engine->api.bestMoves(n, timeMs, depthOrDefault(maxDepth));

        std::string& out = engine->bestMovesBuf;
        out.clear();
        out.reserve(1024);
        out += "{\"moves\":[";
        for (size_t i = 0; i < moves.size(); ++i) {
            if (i != 0) out += ',';
            out += "{\"uci\":";
            appendString(out, moves[i].uci);
            out += ",\"scoreCp\":";
            appendInt(out, moves[i].scoreCp);
            out += ",\"mateIn\":";
            appendInt(out, moves[i].mateIn);
            out += ",\"pv\":";
            appendStringArray(out, moves[i].pv);
            out += '}';
        }
        out += "]}";
        return out.c_str();
    } catch (...) {
        return "{\"moves\":[]}";
    }
}

extern "C" const char* chess_evaluate_json(chess_engine* engine) {
    if (engine == nullptr) return "{\"scoreCp\":0,\"mateIn\":0}";
    try {
        const Evaluation ev = engine->api.evaluate();

        std::string& out = engine->evalBuf;
        out.clear();
        out += "{\"scoreCp\":";
        appendInt(out, ev.scoreCp);
        out += ",\"mateIn\":";
        appendInt(out, ev.mateIn);
        out += '}';
        return out.c_str();
    } catch (...) {
        return "{\"scoreCp\":0,\"mateIn\":0}";
    }
}

extern "C" const char* chess_book_moves_json(chess_engine* engine) {
    if (engine == nullptr) return "{\"moves\":[]}";
    try {
        const std::vector<BookMove> moves = engine->api.bookMoves();

        std::string& out = engine->bookBuf;
        out.clear();
        out += "{\"moves\":[";
        for (size_t i = 0; i < moves.size(); ++i) {
            if (i != 0) out += ',';
            out += "{\"uci\":";
            appendString(out, moves[i].uci);
            out += ",\"weight\":";
            appendInt(out, moves[i].weight);
            out += ",\"percent\":";
            appendInt(out, moves[i].percent);
            out += '}';
        }
        out += "]}";
        return out.c_str();
    } catch (...) {
        return "{\"moves\":[]}";
    }
}

extern "C" void chess_stop(chess_engine* engine) {
    if (engine == nullptr) return;
    engine->api.stop();
}

// ------------------------------------------------------------
//  Canli arama bilgisi -- arama surerken guvenli
// ------------------------------------------------------------

extern "C" int chess_info_depth(chess_engine* engine) {
    return engine != nullptr ? engine->infoDepth.load(std::memory_order_relaxed) : 0;
}

extern "C" int chess_info_sel_depth(chess_engine* engine) {
    return engine != nullptr ? engine->infoSelDepth.load(std::memory_order_relaxed) : 0;
}

extern "C" int chess_info_score_cp(chess_engine* engine) {
    return engine != nullptr ? engine->infoScoreCp.load(std::memory_order_relaxed) : 0;
}

extern "C" int chess_info_mate_in(chess_engine* engine) {
    return engine != nullptr ? engine->infoMateIn.load(std::memory_order_relaxed) : 0;
}

extern "C" long long chess_info_nodes(chess_engine* engine) {
    return engine != nullptr ? engine->infoNodes.load(std::memory_order_relaxed) : 0;
}

extern "C" int chess_info_time_ms(chess_engine* engine) {
    return engine != nullptr ? engine->infoTimeMs.load(std::memory_order_relaxed) : 0;
}

// ------------------------------------------------------------
//  Son aramanin sonucu
// ------------------------------------------------------------

extern "C" int chess_last_score(chess_engine* engine) {
    return engine != nullptr ? engine->api.lastScore() : 0;
}

extern "C" int chess_last_depth(chess_engine* engine) {
    return engine != nullptr ? engine->api.lastDepth() : 0;
}

extern "C" int chess_last_skill_loss(chess_engine* engine) {
    return engine != nullptr ? engine->api.lastSkillLoss() : 0;
}

// ------------------------------------------------------------
//  Ayarlar
// ------------------------------------------------------------

extern "C" void chess_set_skill_level(chess_engine* engine, int level) {
    if (engine == nullptr) return;
    engine->api.setSkillLevel(level);
}

extern "C" int chess_get_skill_level(chess_engine* engine) {
    return engine != nullptr ? engine->api.getSkillLevel() : 0;
}

extern "C" void chess_set_hash_mb(chess_engine* engine, int mb) {
    if (engine == nullptr) return;
    engine->api.setHashSizeMB(mb);
}

extern "C" void chess_set_use_book(chess_engine* engine, int on) {
    if (engine == nullptr) return;
    engine->api.setUseBook(on != 0);
}

extern "C" int chess_load_book_from_memory(chess_engine* engine,
                                           const unsigned char* bytes, size_t size) {
    if (engine == nullptr || bytes == nullptr || size == 0) return 0;
    try {
        return engine->api.loadBookFromMemory(bytes, size) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" int chess_is_book_loaded(chess_engine* engine) {
    return engine != nullptr && engine->api.isBookLoaded() ? 1 : 0;
}
