package com.oguzhanp.chess.engine

// iOS tarafi henuz motora bagli DEGIL. Motoru iOS icin derleyip
// cinterop ile baglamak Faz 1.7'nin isi.
//
// Bu yer tutucu bilerek duruyor: iosArm64 hedefinin derlenebilir
// kalmasini sagliyor, boylece GitHub Actions'taki macOS kosucusu her
// commit'te iOS tarafinin bozulmadigini dogrulayabiliyor.
//
// TODO() cagrisi NotImplementedError firlatir -- sessizce yanlis veri
// dondurmektense gurultuyle durmayi tercih ediyoruz.

actual class ChessEngine actual constructor() {

    actual fun newGame(fen: String): Boolean = TODO("Faz 1.7: cinterop")

    actual fun snapshotJson(): String = TODO("Faz 1.7: cinterop")

    actual fun makeMove(uci: String): Boolean = TODO("Faz 1.7: cinterop")

    actual fun undo(): Boolean = TODO("Faz 1.7: cinterop")

    actual fun sanFor(uci: String): String = TODO("Faz 1.7: cinterop")

    actual fun close() = Unit
}
