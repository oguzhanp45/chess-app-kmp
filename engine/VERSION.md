# Motor kaynak kopyasi / Engine source copy

| | |
|---|---|
| Kaynak / Source | https://github.com/oguzhanp45/cpp-chess-engine |
| Etiket / Tag | `v1.0` |
| Commit | `3655e654c4ab40b4cb9db2f4cf43534f1dfd3393` |
| Kopyalanma / Copied | 2026-08-31 |

**Kopyalanan / Copied:** `src/` (14 dosya), `CMakeLists.txt`
**Kopyalanmayan / Not copied:** `uci/`, `tests/`, `tools/`, `docs/`, `.github/`

Bu klasor ELLE DUZENLENMEZ. Motorda bir degisiklik gerekirse once
cpp-chess-engine deposunda yapilir, yeni bir etiket atilir, sonra bu
klasor bastan kopyalanir ve yukaridaki tablo guncellenir.

C yuzeyi (`chess_c_api`) buraya DEGIL, ayri bir klasore yazilir --
o motorun degil bu uygulamanin parcasi.

Derleme notu: masaustunde her zaman `-DCHESS_BUILD_TOOLS=OFF` ile
yapilandirilir; bu kopyada `tests/` ve `uci/` yok.
