package com.oguzhanp.chess.engine

// iOS tarafi henuz motora bagli DEGIL. Motoru iOS icin derleyip cinterop
// ile baglamak Faz 1.7 sonrasina birakildi.
//
// Bu yer tutucu bilerek duruyor: iosArm64 hedefinin derlenebilir kalmasini
// sagliyor, boylece GitHub Actions'taki macOS kosucusu her commit'te iOS
// tarafinin bozulmadigini dogruluyor.
//
// TODO() NotImplementedError firlatir -- sessizce yanlis veri dondurmektense
// gurultuyle durmayi tercih ediyoruz.

private const val NOT_YET = "iOS: cinterop baglantisi henuz yapilmadi"

internal actual class NativeEngine actual constructor() {

    actual fun newGame(fen: String): Boolean = TODO(NOT_YET)
    actual fun snapshotJson(): String = TODO(NOT_YET)
    actual fun makeMove(uci: String): Boolean = TODO(NOT_YET)
    actual fun undo(): Boolean = TODO(NOT_YET)
    actual fun sanFor(uci: String): String = TODO(NOT_YET)

    actual fun bestMove(timeMs: Int, maxDepth: Int): String = TODO(NOT_YET)
    actual fun bestMovesJson(n: Int, timeMs: Int, maxDepth: Int): String = TODO(NOT_YET)
    actual fun evaluateJson(): String = TODO(NOT_YET)
    actual fun bookMovesJson(): String = TODO(NOT_YET)

    // Bunlar TODO DEGIL: arama surerken cagriliyorlar ve iOS'ta arama
    // hic baslamayacagi icin sifir donmeleri dogru davranis.
    actual fun stop() = Unit
    actual fun infoDepth(): Int = 0
    actual fun infoSelDepth(): Int = 0
    actual fun infoScoreCp(): Int = 0
    actual fun infoMateIn(): Int = 0
    actual fun infoNodes(): Long = 0L
    actual fun infoTimeMs(): Int = 0

    actual fun lastScore(): Int = 0
    actual fun lastDepth(): Int = 0
    actual fun lastSkillLoss(): Int = 0

    actual fun setSkillLevel(level: Int) = Unit
    actual fun getSkillLevel(): Int = 20
    actual fun setHashMb(mb: Int) = Unit
    actual fun setUseBook(on: Boolean) = Unit
    actual fun loadBookFromMemory(bytes: ByteArray): Boolean = false
    actual fun isBookLoaded(): Boolean = false

    actual fun close() = Unit
}
