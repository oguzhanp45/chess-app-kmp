// ============================================================
//  Faz 1.4 -- baglantiyi kanitlama dosyasi
// ============================================================
// Bu dosyanin su anki tek isi: motorun Android icin derlendigini,
// basliklarin bulundugunu ve baglamanin calistigini kanitlamak.
//
// Gercek C yuzeyi (chess_c_api) ve JNI shim'i 1.5'te buraya gelecek;
// o zaman bu fonksiyon silinecek.

#include "EngineApi.hpp"

extern "C" int chess_smoke_test() {
    EngineApi engine;
    // Baslangic pozisyonunda 20 legal hamle vardir.
    return static_cast<int>(engine.legalMoves().size());
}
