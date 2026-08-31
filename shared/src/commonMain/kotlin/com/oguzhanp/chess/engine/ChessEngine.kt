package com.oguzhanp.chess.engine

// ============================================================
//  ChessEngine -- motorun Kotlin yuzu
// ============================================================
// commonMain "ne" oldugunu soyler; her platform "nasil" oldugunu kendi
// kaynak kumesinde yazar. Android JNI ile, iOS cinterop ile baglanir.
// Bu dosyayi kullanan kod ikisini de gormez -- mimari kural 5.
//
// SU AN HAM: snapshotJson() ham JSON metni donduruyor. Onu bir
// Snapshot veri sinifina ayristirmak Faz 2'nin isi (kotlinx.serialization).
// Faz 1'in amaci koprunun calistigini kanitlamak, ustune katman koymak degil.
//
// IS PARCACIGI: alttaki EngineApi is parcacigi guvenli DEGIL. Buradaki
// cagrilarin hepsi ani (arama yok), o yuzden Faz 1'de ana is parcacigindan
// cagrilabilirler. Arama fonksiyonlari geldiginde (Faz 2) coroutine
// sarmalayicisi eklenecek.
//
// OMUR: her ornek C++ tarafinda bir nesne tutuyor. Isi bitince close()
// cagrilmali, yoksa sizinti olur. Compose tarafinda DisposableEffect ile.

expect class ChessEngine() {

    /** Yeni oyun. fen bos ise baslangic pozisyonu. FEN reddedilirse false. */
    fun newGame(fen: String): Boolean

    /** Pozisyonun tam durumu, tek JSON nesnesi olarak. */
    fun snapshotJson(): String

    /** Legal degilse false doner ve tahta degismez. */
    fun makeMove(uci: String): Boolean

    /** Geri alinacak hamle yoksa false. */
    fun undo(): Boolean

    /** Bir hamlenin SAN karsiligi; hamle OYNANMADAN once sorulur. */
    fun sanFor(uci: String): String

    /** C++ tarafindaki nesneyi yok eder. Cagrilmazsa sizinti olur. */
    fun close()
}
