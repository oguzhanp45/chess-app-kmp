#include "chess_c_api.h"
#include "EngineApi.hpp"

#include <exception>

namespace {

// Motor kopyasinin surumu. engine/VERSION.md ile elle esitlenir:
// engine/ klasoru yeni bir etiketten kopyalandiginda burasi da
// guncellenmeli.
const char* const kVersion = "cpp-chess-engine 1.0 (C++17)";

} // namespace

extern "C" const char* chess_version(void) {
    return kVersion;
}

extern "C" int chess_selftest(void) {
    // C sinirindan istisna gecmemeli. EngineApi normalde firlatmaz ama
    // sinir fonksiyonlarinin hepsi bu kalibi kullanacak.
    try {
        EngineApi engine;
        return static_cast<int>(engine.legalMoves().size());
    } catch (...) {
        return -1;
    }
}
