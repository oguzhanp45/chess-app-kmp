package com.oguzhanp.chess.engine

/**
 * libchessjni.so icindeki JNI fonksiyonlarina baglanan TEK nokta.
 *
 * DIKKAT -- buradaki PAKET ADI ve SINIF ADI, chessjni.cpp'deki fonksiyon
 * adlarini belirler:
 *
 *   com.oguzhanp.chess.engine.NativeBridge.makeMove()
 *     -> Java_com_oguzhanp_chess_engine_NativeBridge_makeMove
 *
 * Birini degistirirsen digerini de degistirmek zorundasin. Uyusmazlik
 * DERLEMEDE fark edilmez; calisma aninda UnsatisfiedLinkError olarak
 * ortaya cikar, cunku bag calisma aninda kuruluyor.
 *
 * Uyeler bilerek public: Kotlin `internal` uyeleri JVM bytecode'unda
 * modul adiyla yeniden adlandiriyor, bu da JNI ad eslemesini bozardi.
 * (Kod kucultme acilinca -- Faz 8 -- bu sinif icin keep kurali gerekecek.)
 *
 * handle: chess_create()'in dondurdugu C++ isaretcisi. Kotlin icine
 * bakmaz, yalnizca geri verir.
 */
object NativeBridge {

    init {
        // Kutuphane adi CMake'teki hedef adiyla ayni: chessjni.
        // Android basina "lib", sonuna ".so" ekliyor.
        System.loadLibrary("chessjni")
    }

    // ---- tutamac gerektirmeyenler ----
    external fun version(): String
    external fun selftest(): Int

    // ---- tutamac yonetimi ----
    external fun create(): Long
    external fun destroy(handle: Long)

    // ---- durum ve hamle ----
    external fun newGame(handle: Long, fen: String): Boolean
    external fun snapshotJson(handle: Long): String
    external fun makeMove(handle: Long, uci: String): Boolean
    external fun undo(handle: Long): Boolean
    external fun sanFor(handle: Long, uci: String): String
}
