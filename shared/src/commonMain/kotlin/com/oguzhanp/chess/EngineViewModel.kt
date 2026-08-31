package com.oguzhanp.chess

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.oguzhanp.chess.engine.ChessEngine
import com.oguzhanp.chess.engine.Evaluation
import com.oguzhanp.chess.engine.SearchInfo
import com.oguzhanp.chess.engine.Snapshot
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch

// ============================================================
//  EngineViewModel -- ekran ile motor arasindaki katman
// ============================================================
// Ekran motoru dogrudan tanimaz; bu sinifin akittigi durumu cizer ve
// bu sinifin fonksiyonlarini cagirir. Mimari kural 5.
//
// Faz 3'te tahta gelince ekran degisecek ama buradaki mantik ayni
// kalacak -- amaci da bu.

/** Ekranin cizmek icin ihtiyac duydugu her sey, tek nesnede. */
data class EngineUiState(
    val version: String = "",
    val snapshot: Snapshot = Snapshot(),
    val evaluation: Evaluation = Evaluation(),
    val skillLevel: Int = 20,
    val lastAction: String = "hazir",
)

class EngineViewModel : ViewModel() {

    private val engine = ChessEngine()

    private val _state = MutableStateFlow(EngineUiState())
    val state: StateFlow<EngineUiState> = _state.asStateFlow()

    /** Suren aramanin canli derinlik/skor/dugum bilgisi. */
    val searchInfo: StateFlow<SearchInfo> = engine.searchInfo

    /** Motor su anda dusunuyor mu. */
    val searching: StateFlow<Boolean> = engine.searching

    init {
        viewModelScope.launch {
            _state.update {
                it.copy(version = engine.version, skillLevel = engine.skillLevel())
            }
            refresh()
        }
    }

    // ------------------------------------------------------------
    //  Kullanici eylemleri
    // ------------------------------------------------------------

    fun play(uci: String) {
        viewModelScope.launch {
            // SAN hamle OYNANMADAN once sorulur; sonra sorsak pozisyon
            // degismis olur ve yanlis cevap aliriz.
            val san = engine.sanFor(uci)
            val ok = engine.makeMove(uci)
            setAction(
                if (ok) "makeMove($uci) = true   SAN: $san"
                else "makeMove($uci) = false  -- legal degil"
            )
            refresh()
        }
    }

    fun undo() {
        viewModelScope.launch {
            setAction("undo() = ${engine.undo()}")
            refresh()
        }
    }

    fun newGame() {
        viewModelScope.launch {
            setAction("newGame() = ${engine.newGame()}")
            refresh()
        }
    }

    /**
     * Motor dusunup kendi hamlesini oynar.
     *
     * bestMove BLOKE EDER ama ChessEngine onu kendi dagiticisinda
     * kosturuyor; arayuz donmuyor ve bu sirada searchInfo akiyor.
     */
    fun engineMove(timeMs: Int = 1000) {
        viewModelScope.launch {
            val move = engine.bestMove(timeMs)
            if (move.isEmpty()) {
                setAction("bestMove() bos dondu -- oyun bitmis olabilir")
                return@launch
            }
            val san = engine.sanFor(move)
            engine.makeMove(move)
            setAction("motor oynadi: $move   SAN: $san")
            refresh()
        }
    }

    /** Suren aramayi keser. Siraya girmez, hemen etki eder. */
    fun stopSearch() {
        engine.stop()
    }

    fun setSkillLevel(level: Int) {
        viewModelScope.launch {
            engine.setSkillLevel(level)
            _state.update { it.copy(skillLevel = engine.skillLevel()) }
            setAction("seviye: ${levelName(level)}")
        }
    }

    // ------------------------------------------------------------
    //  Ic isler
    // ------------------------------------------------------------

    private suspend fun refresh() {
        val snapshot = engine.snapshot()
        val evaluation = engine.evaluate()
        _state.update { it.copy(snapshot = snapshot, evaluation = evaluation) }
    }

    private fun setAction(text: String) {
        _state.update { it.copy(lastAction = text) }
    }

    override fun onCleared() {
        engine.close()
    }

    companion object {
        /** Arayuzde sayi degil bu adlar kullanilir. */
        fun levelName(level: Int): String = when {
            level <= 0 -> "Acemi"
            level <= 4 -> "Baslangic"
            level <= 8 -> "Orta"
            level <= 12 -> "Iyi"
            level <= 16 -> "Zor"
            else -> "Tam guc"
        }
    }
}
