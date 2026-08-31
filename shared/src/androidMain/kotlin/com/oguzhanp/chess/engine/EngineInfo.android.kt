package com.oguzhanp.chess.engine

/**
 * libchessjni.so icindeki JNI fonksiyonlarina baglanan tek nokta.
 *
 * DIKKAT -- buradaki PAKET ADI ve SINIF ADI, chessjni.cpp'deki
 * fonksiyon adlarini belirler:
 *
 *   com.oguzhanp.chess.engine.NativeBridge.version()
 *     -> Java_com_oguzhanp_chess_engine_NativeBridge_version
 *
 * Birini degistirirsen digerini de degistirmek zorundasin. Uyusmazlik
 * DERLEMEDE fark edilmez; calisma aninda UnsatisfiedLinkError olarak
 * ortaya cikar, cunku bag calisma aninda kuruluyor.
 *
 * Uyeler bilerek public: Kotlin `internal` uyeleri JVM bytecode'unda
 * modul adiyla yeniden adlandiriyor, bu da JNI ad eslemesini bozardi.
 * (Kod kucultme acilinca -- Faz 8 -- bu sinif icin keep kurali gerekecek.)
 */
object NativeBridge {

    init {
        // Kutuphane adi CMake'teki hedef adiyla ayni: chessjni.
        // Android basina "lib", sonuna ".so" ekliyor.
        System.loadLibrary("chessjni")
    }

    external fun version(): String

    external fun selftest(): Int
}

actual fun engineVersion(): String = NativeBridge.version()

actual fun engineSelfTest(): Int = NativeBridge.selftest()
