package com.oguzhanp.chess.engine

// ============================================================
//  Motor bilgisi -- kopru dogrulama yuzeyi
// ============================================================
// Faz 1.5'in tek amaci: C++ motorunun Kotlin'den okunabildigini
// kanitlamak. Gercek ChessEngine arayuzu Faz 1.6 ve Faz 2'de gelecek;
// bu iki fonksiyon o zaman kaldirilacak.
//
// expect/actual: commonMain "ne" oldugunu soyler, her platform "nasil"
// oldugunu kendi kaynak kumesinde yazar. Android JNI ile, iOS cinterop
// ile baglanacak; App.kt ikisini de bilmez.

/** Motorun surum dizesi. Ornek: "cpp-chess-engine 1.0 (C++17)" */
expect fun engineVersion(): String

/**
 * Kopru oz testi: baslangic pozisyonundaki legal hamle sayisi.
 *   20 = beklenen deger, kopru ve motor calisiyor
 *   -1 = motor kurulamadi
 */
expect fun engineSelfTest(): Int
