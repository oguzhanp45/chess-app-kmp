#include "chess_c_api.h"
#include "EngineApi.hpp"

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

void appendStringArray(std::string& out, const std::vector<std::string>& values) {
    out += '[';
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) out += ',';
        appendString(out, values[i]);
    }
    out += ']';
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
    std::string snapshotBuf;
    std::string sanBuf;
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
