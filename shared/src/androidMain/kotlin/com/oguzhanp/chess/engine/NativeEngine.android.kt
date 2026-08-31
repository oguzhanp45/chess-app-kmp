package com.oguzhanp.chess.engine

internal actual class NativeEngine actual constructor() {

    // C++ tarafindaki nesnenin adresi. 0 = kapatildi.
    private var handle: Long = NativeBridge.create()

    init {
        check(handle != 0L) { "chess_create() basarisiz oldu" }
    }

    // ---- durum ve hamle ----
    actual fun newGame(fen: String): Boolean = NativeBridge.newGame(alive(), fen)
    actual fun snapshotJson(): String = NativeBridge.snapshotJson(alive())
    actual fun makeMove(uci: String): Boolean = NativeBridge.makeMove(alive(), uci)
    actual fun undo(): Boolean = NativeBridge.undo(alive())
    actual fun sanFor(uci: String): String = NativeBridge.sanFor(alive(), uci)

    // ---- arama ----
    actual fun bestMove(timeMs: Int, maxDepth: Int): String =
        NativeBridge.bestMove(alive(), timeMs, maxDepth)

    actual fun bestMovesJson(n: Int, timeMs: Int, maxDepth: Int): String =
        NativeBridge.bestMovesJson(alive(), n, timeMs, maxDepth)

    actual fun evaluateJson(): String = NativeBridge.evaluateJson(alive())
    actual fun bookMovesJson(): String = NativeBridge.bookMovesJson(alive())

    // ---- arama surerken guvenli ----
    // Bunlar alive() KULLANMAZ: arama surerken baska bir is parcacigindan
    // cagriliyorlar ve check() firlatmalari isleri zorlastirir. Tutamac
    // sifirsa C tarafi zaten NULL guvenli davraniyor.
    actual fun stop() = NativeBridge.stop(handle)
    actual fun infoDepth(): Int = NativeBridge.infoDepth(handle)
    actual fun infoSelDepth(): Int = NativeBridge.infoSelDepth(handle)
    actual fun infoScoreCp(): Int = NativeBridge.infoScoreCp(handle)
    actual fun infoMateIn(): Int = NativeBridge.infoMateIn(handle)
    actual fun infoNodes(): Long = NativeBridge.infoNodes(handle)
    actual fun infoTimeMs(): Int = NativeBridge.infoTimeMs(handle)

    // ---- son aramanin sonucu ----
    actual fun lastScore(): Int = NativeBridge.lastScore(alive())
    actual fun lastDepth(): Int = NativeBridge.lastDepth(alive())
    actual fun lastSkillLoss(): Int = NativeBridge.lastSkillLoss(alive())

    // ---- ayarlar ----
    actual fun setSkillLevel(level: Int) = NativeBridge.setSkillLevel(alive(), level)
    actual fun getSkillLevel(): Int = NativeBridge.getSkillLevel(alive())
    actual fun setHashMb(mb: Int) = NativeBridge.setHashMb(alive(), mb)
    actual fun setUseBook(on: Boolean) = NativeBridge.setUseBook(alive(), on)

    actual fun loadBookFromMemory(bytes: ByteArray): Boolean =
        NativeBridge.loadBookFromMemory(alive(), bytes)

    actual fun isBookLoaded(): Boolean = NativeBridge.isBookLoaded(alive())

    actual fun close() {
        if (handle != 0L) {
            NativeBridge.destroy(handle)
            handle = 0L
        }
    }

    // Kapatilmis bir tutamaci C'ye gondermek serbest birakilmis bellege
    // erisim demek -- yani cokme, hem de sebebi anlasilmaz bir cokme.
    // Burada yakalayip anlasilir bir Kotlin hatasina ceviriyoruz.
    private fun alive(): Long {
        check(handle != 0L) { "ChessEngine kapatildi, tekrar kullanilamaz" }
        return handle
    }
}
