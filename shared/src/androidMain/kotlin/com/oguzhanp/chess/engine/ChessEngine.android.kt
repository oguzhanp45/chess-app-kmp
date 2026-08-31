package com.oguzhanp.chess.engine

actual class ChessEngine actual constructor() {

    // C++ tarafindaki nesnenin adresi. 0 = kapatildi.
    private var handle: Long = NativeBridge.create()

    init {
        check(handle != 0L) { "chess_create() basarisiz oldu" }
    }

    actual fun newGame(fen: String): Boolean = NativeBridge.newGame(alive(), fen)

    actual fun snapshotJson(): String = NativeBridge.snapshotJson(alive())

    actual fun makeMove(uci: String): Boolean = NativeBridge.makeMove(alive(), uci)

    actual fun undo(): Boolean = NativeBridge.undo(alive())

    actual fun sanFor(uci: String): String = NativeBridge.sanFor(alive(), uci)

    actual fun close() {
        if (handle != 0L) {
            NativeBridge.destroy(handle)
            handle = 0L
        }
    }

    // Kapatilmis bir tutamaci C'ye gondermek serbest birakilmis bellege
    // erisim demek -- yani cokme. Burada yakalayip anlasilir bir hata
    // veriyoruz.
    private fun alive(): Long {
        check(handle != 0L) { "ChessEngine kapatildi, tekrar kullanilamaz" }
        return handle
    }
}
