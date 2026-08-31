package com.oguzhanp.chess.engine

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

// ============================================================
//  Motorun dondurdugu verinin Kotlin karsiligi
// ============================================================
// C yuzeyi bu yapilari JSON olarak donduruyor; buradaki siniflarin
// alan adlari chess_c_api.cpp'deki yazicinin urettigi adlarla BIREBIR
// ayni olmak zorunda. Bir ad degisirse ayristirma kirilir --
// shared/src/commonTest icindeki testler bunu yakalar.
//
// Bu dosyada satranc KURALI yok, yalnizca motorun soyledigini tasiyan
// veri var. Mimari kural 2 geregi kural uretmiyoruz.

/** Sirasi gelen taraf. */
@Serializable
enum class Side {
    @SerialName("w") WHITE,
    @SerialName("b") BLACK;

    val opposite: Side get() = if (this == WHITE) BLACK else WHITE
}

/**
 * Oyunun durumu. Motorun EngineApi::gameStatus() cikti kumesinin aynisi.
 *
 * CHECKMATE durumunda MAT OLAN taraf Snapshot.side'dir -- yani sirasi
 * gelen taraf kaybetmistir.
 */
@Serializable
enum class GameStatus {
    @SerialName("ongoing") ONGOING,
    @SerialName("checkmate") CHECKMATE,
    @SerialName("stalemate") STALEMATE,
    @SerialName("draw-fifty") DRAW_FIFTY,
    @SerialName("draw-repetition") DRAW_REPETITION,
    @SerialName("draw-material") DRAW_MATERIAL;

    /** Oyun bitti mi. */
    val isOver: Boolean get() = this != ONGOING

    /** Beraberlikle mi bitti. */
    val isDraw: Boolean
        get() = this == STALEMATE || this == DRAW_FIFTY ||
            this == DRAW_REPETITION || this == DRAW_MATERIAL
}

/**
 * Pozisyonun tam durumu -- tek kopru gecisiyle gelir.
 *
 * chess_snapshot_json alti ayri motor sorgusunu tek cagrida topluyor;
 * mimari kural 4'un ("hamle basina bir gecis") uygulanmis hali.
 */
@Serializable
data class Snapshot(
    val fen: String = START_FEN,
    val side: Side = Side.WHITE,
    val inCheck: Boolean = false,
    val status: GameStatus = GameStatus.ONGOING,
    /** Oynanabilecek tum hamleler, UCI: "e2e4", "e7e8q" */
    val legal: List<String> = emptyList(),
    /** Oynanmis hamleler, SAN: "e4", "Nf3", "O-O" */
    val history: List<String> = emptyList(),
) {
    companion object {
        const val START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    }
}

/**
 * Bir pozisyonun degerlendirmesi.
 *
 * scoreCp santipiyon ve SIRA SAHIBINE goredir: pozitif deger, oynayacak
 * olanin lehine demektir. mateIn 0 ise zorunlu mat yok; degilse hamle
 * sayisidir (+ mat ediyor, - mat oluyor). mateIn sifir DEGILKEN scoreCp
 * anlam tasimaz.
 */
@Serializable
data class Evaluation(
    val scoreCp: Int = 0,
    val mateIn: Int = 0,
) {
    val hasMate: Boolean get() = mateIn != 0
}

/** bestMoves'un dondurdugu tek bir aday. pv'nin ilk elemani uci ile aynidir. */
@Serializable
data class ScoredMove(
    val uci: String = "",
    val scoreCp: Int = 0,
    val mateIn: Int = 0,
    /** Beklenen hat (principal variation), UCI olarak. */
    val pv: List<String> = emptyList(),
) {
    val hasMate: Boolean get() = mateIn != 0
}

/** Acilis kitabindaki bir hamle. */
@Serializable
data class BookMove(
    val uci: String = "",
    /** Kitabin kaynak oyunlarindaki ham agirlik. */
    val weight: Int = 0,
    /** Bu pozisyondaki toplam agirligin yuzdesi, 0-100. */
    val percent: Int = 0,
)

/**
 * Suren aramanin anlik durumu. JSON'dan DEGIL, C katmanindaki atomik
 * alanlardan okunur -- bu yuzden @Serializable degil.
 *
 * Arama surerken okunmasi guvenlidir: bu alanlar EngineApi'ye degil,
 * koprunun kendi atomik degiskenlerine bakar.
 */
data class SearchInfo(
    val depth: Int = 0,
    val selDepth: Int = 0,
    val scoreCp: Int = 0,
    val mateIn: Int = 0,
    val nodes: Long = 0,
    val timeMs: Int = 0,
) {
    val hasMate: Boolean get() = mateIn != 0

    /** Saniyede dugum. Sure sifirken 0 doner. */
    val nodesPerSecond: Long get() = if (timeMs > 0) nodes * 1000L / timeMs else 0L

    companion object {
        val IDLE = SearchInfo()
    }
}

// --- ic sarmalayicilar: C tarafi listeleri {"moves":[...]} icinde donduruyor ---

@Serializable
internal data class ScoredMoveList(val moves: List<ScoredMove> = emptyList())

@Serializable
internal data class BookMoveList(val moves: List<BookMove> = emptyList())
