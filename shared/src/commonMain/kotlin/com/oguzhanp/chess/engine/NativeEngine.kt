package com.oguzhanp.chess.engine

// ============================================================
//  NativeEngine -- platform baglantisi, HAM
// ============================================================
// Bu sinif dogrudan kopruye konusur: Android'de JNI, iOS'ta (Faz 1.7
// sonrasi) cinterop. Metotlari suspend DEGIL ve JSON'u ayristirmaz --
// tek isi C fonksiyonlarini cagirmak.
//
// internal: uygulama kodu bunu gormemeli. Disariya bakan yuz
// ChessEngine, ve o coroutine'lidir.
//
// IS PARCACIGI: bu sinif kendi basina hicbir koruma saglamaz. Alttaki
// EngineApi is parcacigi guvenli degildir. Koruma ChessEngine'de,
// tek slotlu dagitici ile yapiliyor.
//
// Tek istisna: stop() ve info* fonksiyonlari. Bunlar motora dokunmaz
// (atomik alanlara bakar) ve arama surerken cagrilabilir.

internal expect class NativeEngine() {

    // ---- durum ve hamle ----
    fun newGame(fen: String): Boolean
    fun snapshotJson(): String
    fun makeMove(uci: String): Boolean
    fun undo(): Boolean
    fun sanFor(uci: String): String

    // ---- arama: BLOKE EDER ----
    fun bestMove(timeMs: Int, maxDepth: Int): String
    fun bestMovesJson(n: Int, timeMs: Int, maxDepth: Int): String
    fun evaluateJson(): String
    fun bookMovesJson(): String

    // ---- arama surerken guvenli ----
    fun stop()
    fun infoDepth(): Int
    fun infoSelDepth(): Int
    fun infoScoreCp(): Int
    fun infoMateIn(): Int
    fun infoNodes(): Long
    fun infoTimeMs(): Int

    // ---- son aramanin sonucu ----
    fun lastScore(): Int
    fun lastDepth(): Int
    fun lastSkillLoss(): Int

    // ---- ayarlar ----
    fun setSkillLevel(level: Int)
    fun getSkillLevel(): Int
    fun setHashMb(mb: Int)
    fun setUseBook(on: Boolean)
    fun loadBookFromMemory(bytes: ByteArray): Boolean
    fun isBookLoaded(): Boolean

    fun close()
}
