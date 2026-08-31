// ============================================================
//  capitest -- chess_c_api'nin masaustu testi
// ============================================================
// C yuzeyini cihaza gitmeden dogrular. Motorun kendi test takimi
// (cpp-chess-engine deposundaki apitest) motoru olcuyor; bu dosya
// KOPRUYU olcuyor: JSON bicimi, tutamac omru, NULL davranisi,
// tampon ayriligi.
//
// Calistirma (Git Bash, depo kokunde):
//   cmake -B build-desktop -DCHESS_BUILD_TOOLS=OFF
//   cmake --build build-desktop --config Release
//   ./build-desktop/Release/capitest.exe        (Windows)
//   ./build-desktop/capitest                    (Linux/macOS)
//
// Cikis kodu: 0 = hepsi gecti, 1 = en az bir kontrol basarisiz.

#include "chess_c_api.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

namespace {

int checks = 0;
int failures = 0;

void check(const char* name, bool ok) {
    ++checks;
    if (!ok) ++failures;
    std::printf("%-44s %s\n", name, ok ? "gecti" : "BASARISIZ");
}

// Ham JSON icinde bir alt dize var mi. Tam bir ayristirici yazmiyoruz;
// bicimin dogru oldugunu gormek icin bu yeterli.
bool contains(const char* haystack, const char* needle) {
    return std::strstr(haystack, needle) != nullptr;
}

} // namespace

int main() {
    std::printf("chess_version : %s\n", chess_version());
    std::printf("chess_selftest: %d\n\n", chess_selftest());

    // ---------- surum ve oz test ----------
    check("chess_version bos degil", std::strlen(chess_version()) > 0);
    check("chess_selftest == 20", chess_selftest() == 20);

    // ---------- tutamac ----------
    chess_engine* e = chess_create();
    check("chess_create NULL degil", e != nullptr);
    if (e == nullptr) return 1;

    // ---------- baslangic snapshot ----------
    const char* snap = chess_snapshot_json(e);
    check("baslangic FEN dogru",
          contains(snap, "\"fen\":\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\""));
    check("baslangic side w",      contains(snap, "\"side\":\"w\""));
    check("baslangic inCheck false", contains(snap, "\"inCheck\":false"));
    check("baslangic status ongoing", contains(snap, "\"status\":\"ongoing\""));
    check("legal listesinde e2e4 var", contains(snap, "\"e2e4\""));
    check("history bos",           contains(snap, "\"history\":[]"));

    // ---------- SAN ----------
    check("sanFor(e2e4) == e4",   std::strcmp(chess_san_for(e, "e2e4"), "e4") == 0);
    check("sanFor(g1f3) == Nf3",  std::strcmp(chess_san_for(e, "g1f3"), "Nf3") == 0);
    check("sanFor(legal olmayan) bos",
          std::strlen(chess_san_for(e, "e2e5")) == 0);

    // ---------- tampon ayriligi ----------
    // snapshot isaretcisi elde tutulurken san_for cagrilirsa snapshot
    // BOZULMAMALI. Ayri tamponlar bunun icin var.
    const char* held = chess_snapshot_json(e);
    std::string before = held;
    chess_san_for(e, "d2d4");
    check("san_for snapshot tamponunu bozmuyor", before == held);

    // ---------- hamle ----------
    check("makeMove(e2e4)",           chess_make_move(e, "e2e4") == 1);
    check("makeMove(e7e5)",           chess_make_move(e, "e7e5") == 1);
    check("makeMove(legal olmayan) 0", chess_make_move(e, "e2e5") == 0);

    snap = chess_snapshot_json(e);
    check("1.e4 e5 sonrasi FEN dogru",
          contains(snap, "\"fen\":\"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2\""));
    check("history [e4,e5]", contains(snap, "\"history\":[\"e4\",\"e5\"]"));

    // ---------- undo ----------
    check("undo", chess_undo(e) == 1);
    snap = chess_snapshot_json(e);
    check("undo sonrasi sira siyahta", contains(snap, "\"side\":\"b\""));
    check("undo sonrasi history [e4]", contains(snap, "\"history\":[\"e4\"]"));

    // ---------- mat ----------
    // Arka sira mati: 6k1/5ppp/8/8/8/8/8/4R1K1 w - - 0 1, Re8#
    check("new_game(FEN)",
          chess_new_game(e, "6k1/5ppp/8/8/8/8/8/4R1K1 w - - 0 1") == 1);
    check("makeMove(e1e8)", chess_make_move(e, "e1e8") == 1);
    snap = chess_snapshot_json(e);
    check("status checkmate",  contains(snap, "\"status\":\"checkmate\""));
    check("mat olan taraf siyah", contains(snap, "\"side\":\"b\""));
    check("mat pozisyonunda legal hamle yok", contains(snap, "\"legal\":[]"));
    check("mat pozisyonunda inCheck true", contains(snap, "\"inCheck\":true"));

    // ---------- pat ----------
    check("new_game(pat FEN)",
          chess_new_game(e, "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1") == 1);
    snap = chess_snapshot_json(e);
    check("status stalemate", contains(snap, "\"status\":\"stalemate\""));

    // ---------- gecersiz FEN ----------
    check("gecersiz FEN reddedilir", chess_new_game(e, "bu bir fen degil") == 0);
    snap = chess_snapshot_json(e);
    check("gecersiz FEN sonrasi baslangic pozisyonu",
          contains(snap, "\"fen\":\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\""));

    // ---------- degerlendirme ----------
    check("new_game(baslangic)", chess_new_game(e, "") == 1);
    const char* ev = chess_evaluate_json(e);
    check("evaluate bicimi dogru",
          contains(ev, "\"scoreCp\":") && contains(ev, "\"mateIn\":"));
    check("baslangicta zorunlu mat yok", contains(ev, "\"mateIn\":0"));

    // ---------- acilis kitabi ----------
    // Kitap yuklenmedi; gezgin bos donmeli, cokmemeli.
    check("kitap yuklu degil", chess_is_book_loaded(e) == 0);
    check("kitapsiz book_moves bos", contains(chess_book_moves_json(e), "\"moves\":[]"));

    // ---------- ayarlar ----------
    chess_set_skill_level(e, 8);
    check("skill level 8 yazilip okundu", chess_get_skill_level(e) == 8);
    chess_set_skill_level(e, 20);
    check("skill level 20 yazilip okundu", chess_get_skill_level(e) == 20);
    chess_set_hash_mb(e, 16);      // cokmemeli
    chess_set_use_book(e, 0);      // cokmemeli

    // ---------- arama ----------
    const std::string best = chess_best_move(e, 300, 0);
    check("best_move bir hamle dondurdu", best.size() >= 4);
    check("best_move legal listede",
          contains(chess_snapshot_json(e), ("\"" + best + "\"").c_str()));

    check("info depth > 0",  chess_info_depth(e) > 0);
    check("info nodes > 0",  chess_info_nodes(e) > 0);
    check("last_depth > 0",  chess_last_depth(e) > 0);

    // ---------- multi-PV ----------
    const char* multi = chess_best_moves_json(e, 3, 300, 0);
    check("best_moves bicimi dogru",
          contains(multi, "\"moves\":[{\"uci\":") &&
          contains(multi, "\"scoreCp\":") &&
          contains(multi, "\"pv\":["));

    // ---------- mat tespiti ----------
    // 6k1/5ppp/8/8/8/8/8/4R1K1 w - - 0 1 -> Re8 mat.
    check("new_game(mat-1 FEN)",
          chess_new_game(e, "6k1/5ppp/8/8/8/8/8/4R1K1 w - - 0 1") == 1);
    const char* mateLine = chess_best_moves_json(e, 1, 500, 0);
    check("mat-1 bulundu (mateIn 1)", contains(mateLine, "\"mateIn\":1"));
    check("mat hamlesi e1e8", contains(mateLine, "\"uci\":\"e1e8\""));

    // ---------- stop baska is parcacigindan ----------
    // Motorun tek is parcacigi guvenli olmayan yuzeyinde stop() istisnadir.
    // 5 saniyelik bir arama 200 ms sonra kesilmeli.
    chess_new_game(e, "");
    {
        const auto started = std::chrono::steady_clock::now();
        std::thread worker([&] { chess_best_move(e, 5000, 0); });
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        chess_stop(e);
        worker.join();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started).count();
        std::printf("  (arama %lld ms sonra kesildi)\n", static_cast<long long>(elapsedMs));
        check("stop aramayi kesti (< 2000 ms)", elapsedMs < 2000);
    }

    // ---------- NULL guvenligi ----------
    chess_destroy(e);
    chess_destroy(nullptr);
    check("NULL tutamac snapshot -> {}",
          std::strcmp(chess_snapshot_json(nullptr), "{}") == 0);
    check("NULL tutamac make_move -> 0", chess_make_move(nullptr, "e2e4") == 0);
    check("NULL tutamac undo -> 0",      chess_undo(nullptr) == 0);
    check("NULL uci san_for -> bos",     std::strlen(chess_san_for(nullptr, nullptr)) == 0);
    check("NULL best_move -> bos",       std::strlen(chess_best_move(nullptr, 10, 0)) == 0);
    check("NULL best_moves -> bos liste",
          std::strcmp(chess_best_moves_json(nullptr, 3, 10, 0), "{\"moves\":[]}") == 0);
    check("NULL info_depth -> 0",        chess_info_depth(nullptr) == 0);
    chess_stop(nullptr);   // cokmemeli

    std::printf("\ncapitest %d/%d\n", checks - failures, checks);
    return failures == 0 ? 0 : 1;
}
