package com.oguzhanp.chess.engine

import kotlinx.serialization.json.Json

/**
 * C yuzeyinden gelen JSON metinlerini veri siniflarina cevirir.
 *
 * ignoreUnknownKeys: C tarafina ileride yeni bir alan eklenirse eski
 * Kotlin kodu kirilmasin diye. Ters yon (Kotlin'de olup C'de olmayan
 * alan) veri siniflarindaki varsayilan degerlerle karsilaniyor.
 *
 * Bu nesne INTERNAL: ekran kodu JSON diye bir sey bilmemeli, yalnizca
 * ChessEngine'in dondurdugu veri siniflarini gormeli.
 */
internal object EngineJson {

    private val json = Json {
        ignoreUnknownKeys = true
    }

    fun snapshot(text: String): Snapshot = json.decodeFromString(text)

    fun evaluation(text: String): Evaluation = json.decodeFromString(text)

    fun scoredMoves(text: String): List<ScoredMove> =
        json.decodeFromString<ScoredMoveList>(text).moves

    fun bookMoves(text: String): List<BookMove> =
        json.decodeFromString<BookMoveList>(text).moves
}
