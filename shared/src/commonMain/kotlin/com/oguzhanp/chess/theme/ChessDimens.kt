package com.oguzhanp.chess.theme

import androidx.compose.ui.unit.dp

// ============================================================
//  Olcu sistemi -- bosluklar ve kose yaricaplari
// ============================================================
// Mimari kural 6. Renkler ChessColors'ta, olculer burada.
//
// Amac tutarlilik: her ekranda ayni bosluk degerleri kullanilirsa
// uygulama tek elden cikmis gibi durur. Elle 18.dp, 22.dp yazmak
// tek basina yanlis degildir ama bes ekran sonra hicbir sey
// hizalanmaz.
//
// Olcek 4'un katlari: 4 - 8 - 16 - 24 - 32 - 48. Ara degerler
// (12, 20) bilerek yok; secim ne kadar azsa tutarlilik o kadar kolay.
//
// NOT: bunlar sabit. Ileride ekran genisligine gore degismeleri
// gerekirse (Faz 5'te tablet duzeni) bir CompositionLocal'a
// tasinacaklar; kullanim yerleri degismeyecek.

object Spacing {
    /** 4dp -- birbirine ait iki oge arasi (ikon ve etiketi gibi). */
    val xs = 4.dp

    /** 8dp -- ayni gruptaki ogeler arasi. */
    val sm = 8.dp

    /** 16dp -- gruplar arasi, kart ici dolgu. */
    val md = 16.dp

    /** 24dp -- bolumler arasi. Ekran kenar boslugu da bu. */
    val lg = 24.dp

    /** 32dp -- buyuk ayrimlar. */
    val xl = 32.dp

    /** 48dp -- gorsel ile metin arasi gibi nefes alan yerler. */
    val xxl = 48.dp

    /** Ekranin kenarlarindan icerideki bosluk. */
    val screen = 24.dp
}

object Corners {
    /** 6dp -- kucuk ogeler: rozet, cubuk ucu. */
    val sm = 6.dp

    /** 12dp -- tuslar, giris alanlari. */
    val md = 12.dp

    /** 20dp -- kartlar, panolar. */
    val lg = 20.dp
}
