package com.oguzhanp.chess.engine

import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertFalse
import kotlin.test.assertTrue

/**
 * Ayristirmanin C tarafinin gercekten urettigi bicimle uyumlu oldugunu
 * dogrular.
 *
 * Buradaki JSON metinleri UYDURULMADI: capitest calistirilip ciktisindan
 * alindi. Alan adlarindan biri C tarafinda degisirse bu testler kirilir --
 * amac tam olarak bu.
 */
class EngineJsonTest {

    @Test
    fun baslangicPozisyonuAyristirilir() {
        val text = """
            {"fen":"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
             "side":"w","inCheck":false,"status":"ongoing",
             "legal":["a2a3","a2a4","b1a3","g1f3"],"history":[]}
        """.trimIndent()

        val snapshot = EngineJson.snapshot(text)

        assertEquals(Snapshot.START_FEN, snapshot.fen)
        assertEquals(Side.WHITE, snapshot.side)
        assertFalse(snapshot.inCheck)
        assertEquals(GameStatus.ONGOING, snapshot.status)
        assertEquals(4, snapshot.legal.size)
        assertTrue(snapshot.history.isEmpty())
        assertFalse(snapshot.status.isOver)
    }

    @Test
    fun matDurumuAyristirilir() {
        // capitest'teki arka sira mati: Re8#
        val text = """
            {"fen":"4R1k1/5ppp/8/8/8/8/8/6K1 b - - 1 1","side":"b",
             "inCheck":true,"status":"checkmate","legal":[],"history":["Re8#"]}
        """.trimIndent()

        val snapshot = EngineJson.snapshot(text)

        assertEquals(GameStatus.CHECKMATE, snapshot.status)
        // Mat olan taraf sirasi gelen taraftir.
        assertEquals(Side.BLACK, snapshot.side)
        assertTrue(snapshot.inCheck)
        assertTrue(snapshot.legal.isEmpty())
        assertTrue(snapshot.status.isOver)
        assertFalse(snapshot.status.isDraw)
    }

    @Test
    fun beraberlikDurumlariTaninir() {
        val statuses = mapOf(
            "stalemate" to GameStatus.STALEMATE,
            "draw-fifty" to GameStatus.DRAW_FIFTY,
            "draw-repetition" to GameStatus.DRAW_REPETITION,
            "draw-material" to GameStatus.DRAW_MATERIAL,
        )

        for ((text, expected) in statuses) {
            val snapshot = EngineJson.snapshot(
                """{"fen":"8/8/8/8/8/8/8/8 w - - 0 1","side":"w","inCheck":false,
                   "status":"$text","legal":[],"history":[]}"""
            )
            assertEquals(expected, snapshot.status, "status: $text")
            assertTrue(snapshot.status.isDraw, "isDraw: $text")
        }
    }

    @Test
    fun degerlendirmeAyristirilir() {
        val ev = EngineJson.evaluation("""{"scoreCp":34,"mateIn":0}""")
        assertEquals(34, ev.scoreCp)
        assertEquals(0, ev.mateIn)
        assertFalse(ev.hasMate)

        val mate = EngineJson.evaluation("""{"scoreCp":0,"mateIn":-3}""")
        assertTrue(mate.hasMate)
        assertEquals(-3, mate.mateIn)
    }

    @Test
    fun coklVaryantAyristirilir() {
        val text = """
            {"moves":[
              {"uci":"e2e4","scoreCp":34,"mateIn":0,"pv":["e2e4","e7e5","g1f3"]},
              {"uci":"d2d4","scoreCp":28,"mateIn":0,"pv":["d2d4","d7d5"]}
            ]}
        """.trimIndent()

        val moves = EngineJson.scoredMoves(text)

        assertEquals(2, moves.size)
        assertEquals("e2e4", moves[0].uci)
        assertEquals(34, moves[0].scoreCp)
        assertEquals(3, moves[0].pv.size)
        // pv'nin ilk elemani hamlenin kendisidir.
        assertEquals(moves[0].uci, moves[0].pv.first())
        // Skora gore azalan sirada gelir.
        assertTrue(moves[0].scoreCp >= moves[1].scoreCp)
    }

    @Test
    fun matVaryantiAyristirilir() {
        val moves = EngineJson.scoredMoves(
            """{"moves":[{"uci":"e1e8","scoreCp":0,"mateIn":1,"pv":["e1e8"]}]}"""
        )
        assertEquals(1, moves.size)
        assertTrue(moves[0].hasMate)
        assertEquals(1, moves[0].mateIn)
    }

    @Test
    fun bosListelerAyristirilir() {
        assertTrue(EngineJson.scoredMoves("""{"moves":[]}""").isEmpty())
        assertTrue(EngineJson.bookMoves("""{"moves":[]}""").isEmpty())
    }

    @Test
    fun kitapHamleleriAyristirilir() {
        val moves = EngineJson.bookMoves(
            """{"moves":[{"uci":"e2e4","weight":8000,"percent":42}]}"""
        )
        assertEquals(1, moves.size)
        assertEquals("e2e4", moves[0].uci)
        assertEquals(8000, moves[0].weight)
        assertEquals(42, moves[0].percent)
    }

    @Test
    fun bilinmeyenAlanlarYokSayilir() {
        // C tarafina ileride yeni bir alan eklenirse eski Kotlin kirilmamali.
        val snapshot = EngineJson.snapshot(
            """{"fen":"${Snapshot.START_FEN}","side":"w","inCheck":false,
               "status":"ongoing","legal":[],"history":[],"yeniAlan":123}"""
        )
        assertEquals(Side.WHITE, snapshot.side)
    }

    @Test
    fun sideOppositeCalisir() {
        assertEquals(Side.BLACK, Side.WHITE.opposite)
        assertEquals(Side.WHITE, Side.BLACK.opposite)
    }

    @Test
    fun searchInfoNpsHesaplar() {
        assertEquals(0L, SearchInfo.IDLE.nodesPerSecond)
        assertEquals(500_000L, SearchInfo(nodes = 1_000_000, timeMs = 2000).nodesPerSecond)
    }
}
