package com.oguzhanp.chess.engine

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

// ============================================================
//  ChessEngine -- motorun disariya bakan yuzu
// ============================================================
// IS PARCACIGI GUVENLIGI YAPISALDIR
//
// Alttaki EngineApi is parcacigi guvenli degil: arama surerken
// makeMove() cagirmak hata vermez, SESSIZCE yanlis cevap verir.
// Motor fazinda bu belgelenmisti; RN'de disiplinle korunuyordu.
//
// Burada disiplin yerine tip sistemi koruyor: butun motor cagrilari
// TEK SLOTLU bir dagiticida kosuyor. Arama surerken gelen bir cagri
// siraya girer, araya giremez. Kural 3 artik ihlal edilemez.
//
// Iki istisna suspend DEGIL, cunku siraya girmemeleri gerekiyor:
//   stop()         -- siraya girse aramanin bitmesini beklerdi, anlamsiz
//   currentInfo()  -- arama surerken okunmali
// Ikisi de motora dokunmuyor: stop atomik bir bayrak set ediyor,
// currentInfo koprunun atomik alanlarini okuyor.

class ChessEngine {

    private val native = NativeEngine()

    // Tek slot: ayni anda en fazla bir cagri motora ulasir.
    // limitedParallelism(1) kotlinx.coroutines 1.9'dan beri kararli.
    private val engineContext = Dispatchers.Default.limitedParallelism(1)

    // close() suren islerin arkasina siralanabilsin diye kendi kapsami.
    private val scope = CoroutineScope(engineContext + SupervisorJob())

    private val _searchInfo = MutableStateFlow(SearchInfo.IDLE)

    /** Suren aramanin canli durumu. Arama yokken son degerde kalir. */
    val searchInfo: StateFlow<SearchInfo> = _searchInfo.asStateFlow()

    private val _searching = MutableStateFlow(false)

    /** Su anda bir arama suruyor mu. */
    val searching: StateFlow<Boolean> = _searching.asStateFlow()

    /** Motorun surum dizesi. Motora dokunmaz, sabit bir metin. */
    val version: String get() = engineVersion()

    // ------------------------------------------------------------
    //  Durum ve hamle -- hepsi ani
    // ------------------------------------------------------------

    suspend fun newGame(fen: String = ""): Boolean = onEngine { native.newGame(fen) }

    suspend fun snapshot(): Snapshot = onEngine { EngineJson.snapshot(native.snapshotJson()) }

    suspend fun makeMove(uci: String): Boolean = onEngine { native.makeMove(uci) }

    suspend fun undo(): Boolean = onEngine { native.undo() }

    /** Bir hamlenin SAN karsiligi. Hamle OYNANMADAN once sorulur. */
    suspend fun sanFor(uci: String): String = onEngine { native.sanFor(uci) }

    suspend fun evaluate(): Evaluation = onEngine {
        EngineJson.evaluation(native.evaluateJson())
    }

    suspend fun bookMoves(): List<BookMove> = onEngine {
        EngineJson.bookMoves(native.bookMovesJson())
    }

    // ------------------------------------------------------------
    //  Arama -- uzun surer, canli bilgi akar
    // ------------------------------------------------------------

    /**
     * Motorun oynayacagi hamle. Seviye ayarina ve acilis kitabina uyar.
     * maxDepth 0 verilirse sinirsiz.
     */
    suspend fun bestMove(timeMs: Int, maxDepth: Int = 0): String =
        withLiveInfo { native.bestMove(timeMs, maxDepth) }

    /**
     * En iyi n hamle, skorlari ve varyantlariyla. ANALIZ icindir:
     * seviye ayari ve kitap yok sayilir, arama her zaman tam gucte yapilir.
     * Ayni derinlikte bestMove'dan 2-4 kat yavastir.
     */
    suspend fun bestMoves(n: Int, timeMs: Int, maxDepth: Int = 0): List<ScoredMove> =
        withLiveInfo { EngineJson.scoredMoves(native.bestMovesJson(n, timeMs, maxDepth)) }

    /** Suren aramayi keser. Siraya GIRMEZ, hemen etki eder. */
    fun stop() {
        native.stop()
    }

    /** Aninda okunur; arama surerken cagrilmasi guvenlidir. */
    fun currentInfo(): SearchInfo = readInfo()

    // ------------------------------------------------------------
    //  Ayarlar
    // ------------------------------------------------------------

    /** 0 (en zayif) - 20 (tam guc). bestMove'u etkiler, bestMoves'u asla. */
    suspend fun setSkillLevel(level: Int) = onEngine { native.setSkillLevel(level) }

    suspend fun skillLevel(): Int = onEngine { native.getSkillLevel() }

    suspend fun setHashMb(mb: Int) = onEngine { native.setHashMb(mb) }

    /** Motorun OYNARKEN kitabi kullanip kullanmadigi. */
    suspend fun setUseBook(on: Boolean) = onEngine { native.setUseBook(on) }

    suspend fun loadBook(bytes: ByteArray): Boolean = onEngine {
        native.loadBookFromMemory(bytes)
    }

    suspend fun isBookLoaded(): Boolean = onEngine { native.isBookLoaded() }

    // ------------------------------------------------------------
    //  Omur
    // ------------------------------------------------------------

    /**
     * C++ tarafindaki nesneyi yok eder. Once suren aramayi kesiyor,
     * sonra yok etme isini siraya koyuyor -- boylece arama ortasinda
     * nesne silinmiyor.
     */
    fun close() {
        native.stop()
        scope.launch { native.close() }
    }

    // ------------------------------------------------------------
    //  Ic yardimcilar
    // ------------------------------------------------------------

    private suspend fun <T> onEngine(block: () -> T): T =
        withContext(engineContext) { block() }

    /**
     * Bloke eden bir aramayi kosturur ve bu sirada canli bilgiyi akitir.
     *
     * Yoklayici BASKA bir dagiticida kosuyor -- engineContext'te kossaydi
     * aramanin arkasina siralanir ve arama bitene kadar hic calismazdi.
     * Okudugu alanlar atomik oldugu icin bu guvenli.
     */
    private suspend fun <T> withLiveInfo(block: () -> T): T = coroutineScope {
        _searchInfo.value = SearchInfo.IDLE
        _searching.value = true

        val poller = launch(Dispatchers.Default) {
            while (isActive) {
                _searchInfo.value = readInfo()
                delay(POLL_INTERVAL_MS)
            }
        }

        try {
            withContext(engineContext) { block() }
        } finally {
            poller.cancel()
            _searchInfo.value = readInfo()   // son deger kalsin
            _searching.value = false
        }
    }

    private fun readInfo() = SearchInfo(
        depth = native.infoDepth(),
        selDepth = native.infoSelDepth(),
        scoreCp = native.infoScoreCp(),
        mateIn = native.infoMateIn(),
        nodes = native.infoNodes(),
        timeMs = native.infoTimeMs(),
    )

    private companion object {
        // Arayuz icin 100 ms yeterli; daha sik yoklamak bosuna is.
        const val POLL_INTERVAL_MS = 100L
    }
}
