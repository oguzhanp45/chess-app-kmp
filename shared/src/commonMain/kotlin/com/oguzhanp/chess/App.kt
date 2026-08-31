package com.oguzhanp.chess

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeContentPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel

import com.oguzhanp.chess.engine.Evaluation
import com.oguzhanp.chess.engine.GameStatus
import com.oguzhanp.chess.engine.SearchInfo
import com.oguzhanp.chess.engine.Side
import com.oguzhanp.chess.engine.Snapshot

// ============================================================
//  Faz 2.4 -- motor dogrulama ekrani
// ============================================================
// Bu ekran GECICIDIR ve tasarim degildir. Tek isi motorun coroutine
// sarmalayicisinin dogru calistigini gostermek: motor dusunurken arayuz
// donmuyor mu, canli derinlik akiyor mu, durdurma calisiyor mu.
//
// Gercek arayuz Faz 3'te (tahta) ve Faz 5'te (navigasyon) yapilacak;
// oraya gelmeden once tasarim sorulari sorulacak.
//
// Yapi: App() ViewModel'i baglar, AppContent() yalnizca aldigini cizer.

@Composable
fun App() {
    MaterialTheme {
        val viewModel: EngineViewModel = viewModel { EngineViewModel() }

        val state by viewModel.state.collectAsStateWithLifecycle()
        val info by viewModel.searchInfo.collectAsStateWithLifecycle()
        val searching by viewModel.searching.collectAsStateWithLifecycle()

        var moveText by remember { mutableStateOf("e2e4") }

        AppContent(
            state = state,
            info = info,
            searching = searching,
            moveText = moveText,
            onMoveTextChange = { moveText = it.trim() },
            onPlay = { viewModel.play(moveText) },
            onUndo = viewModel::undo,
            onNewGame = viewModel::newGame,
            onEngineMove = { viewModel.engineMove(timeMs = 1000) },
            onStop = viewModel::stopSearch,
            onLevel = viewModel::setSkillLevel,
        )
    }
}

@Composable
fun AppContent(
    state: EngineUiState,
    info: SearchInfo,
    searching: Boolean,
    moveText: String,
    onMoveTextChange: (String) -> Unit,
    onPlay: () -> Unit,
    onUndo: () -> Unit,
    onNewGame: () -> Unit,
    onEngineMove: () -> Unit,
    onStop: () -> Unit,
    onLevel: (Int) -> Unit,
) {
    val snapshot = state.snapshot

    Column(
        modifier = Modifier
            .safeContentPadding()
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
    ) {
        Text(state.version, style = MaterialTheme.typography.titleMedium)

        Spacer(Modifier.height(16.dp))

        // ---- pozisyon ----
        Field("Sira", if (snapshot.side == Side.WHITE) "beyaz" else "siyah")
        Field("Durum", statusText(snapshot.status))
        Field("Sah altinda", if (snapshot.inCheck) "evet" else "hayir")
        Field("Legal hamle", snapshot.legal.size.toString())
        Field("Degerlendirme", evalText(state.evaluation))
        Field("Seviye", "${EngineViewModel.levelName(state.skillLevel)} (${state.skillLevel})")

        Spacer(Modifier.height(12.dp))

        Text("FEN", style = MaterialTheme.typography.labelMedium)
        Text(snapshot.fen, style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace)

        Spacer(Modifier.height(12.dp))

        Text("Hamleler", style = MaterialTheme.typography.labelMedium)
        Text(
            text = if (snapshot.history.isEmpty()) "-" else snapshot.history.joinToString(" "),
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
        )

        Spacer(Modifier.height(20.dp))

        // ---- elle hamle ----
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            OutlinedTextField(
                value = moveText,
                onValueChange = onMoveTextChange,
                label = { Text("UCI hamle") },
                singleLine = true,
                enabled = !searching,
                modifier = Modifier.weight(1f),
            )
            Button(onClick = onPlay, enabled = !searching) { Text("Oyna") }
        }

        Spacer(Modifier.height(8.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = onUndo, enabled = !searching) { Text("Geri al") }
            OutlinedButton(onClick = onNewGame, enabled = !searching) { Text("Yeni oyun") }
        }

        Spacer(Modifier.height(16.dp))

        // ---- motor ----
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = onEngineMove, enabled = !searching) { Text("Motor oynasin") }
            Button(onClick = onStop, enabled = searching) { Text("Durdur") }
        }

        Spacer(Modifier.height(8.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { onLevel(0) }, enabled = !searching) { Text("Acemi") }
            OutlinedButton(onClick = { onLevel(8) }, enabled = !searching) { Text("Orta") }
            OutlinedButton(onClick = { onLevel(20) }, enabled = !searching) { Text("Tam guc") }
        }

        Spacer(Modifier.height(16.dp))

        // ---- canli arama bilgisi ----
        Text(
            text = if (searching) "Motor dusunuyor..." else "Son arama",
            style = MaterialTheme.typography.labelMedium,
        )
        Text(
            text = infoText(info),
            style = MaterialTheme.typography.bodyMedium,
            fontFamily = FontFamily.Monospace,
        )

        Spacer(Modifier.height(16.dp))

        Text("Son islem", style = MaterialTheme.typography.labelMedium)
        Text(
            text = state.lastAction,
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
        )
    }
}

@Composable
private fun Field(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = label,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.weight(0.4f),
        )
        Text(
            text = value,
            style = MaterialTheme.typography.bodyMedium,
            fontFamily = FontFamily.Monospace,
            modifier = Modifier.weight(0.6f),
        )
    }
}

// Bu metinler Faz 6'da sozluge tasinacak (mimari kural 7).
private fun statusText(status: GameStatus): String = when (status) {
    GameStatus.ONGOING -> "devam ediyor"
    GameStatus.CHECKMATE -> "MAT"
    GameStatus.STALEMATE -> "pat"
    GameStatus.DRAW_FIFTY -> "beraberlik (50 hamle)"
    GameStatus.DRAW_REPETITION -> "beraberlik (tekrar)"
    GameStatus.DRAW_MATERIAL -> "beraberlik (yetersiz materyal)"
}

private fun evalText(evaluation: Evaluation): String =
    if (evaluation.hasMate) "mat ${evaluation.mateIn}"
    else "${evaluation.scoreCp} cp"

private fun infoText(info: SearchInfo): String {
    if (info.depth == 0 && info.nodes == 0L) return "-"
    val score = if (info.hasMate) "mat ${info.mateIn}" else "${info.scoreCp} cp"
    return "derinlik ${info.depth}/${info.selDepth}  $score  " +
        "${info.nodes} dugum  ${info.timeMs} ms  ${info.nodesPerSecond} d/sn"
}

// ------------------------------------------------------------
//  Onizlemeler -- ViewModel'e hic dokunmadan
// ------------------------------------------------------------

private val PREVIEW_STATE = EngineUiState(
    version = "cpp-chess-engine 1.0 (C++17)",
    snapshot = Snapshot(
        fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
        side = Side.WHITE,
        status = GameStatus.ONGOING,
        legal = List(29) { "" },
        history = listOf("e4", "e5"),
    ),
    evaluation = Evaluation(scoreCp = 34),
    lastAction = "motor oynadi: g1f3   SAN: Nf3",
)

@Preview(showBackground = true)
@Composable
private fun AppContentIdlePreview() {
    MaterialTheme {
        AppContent(
            state = PREVIEW_STATE,
            info = SearchInfo(depth = 12, selDepth = 18, scoreCp = 34,
                nodes = 412_000, timeMs = 1000),
            searching = false,
            moveText = "e2e4",
            onMoveTextChange = {}, onPlay = {}, onUndo = {}, onNewGame = {},
            onEngineMove = {}, onStop = {}, onLevel = {},
        )
    }
}

@Preview(showBackground = true)
@Composable
private fun AppContentSearchingPreview() {
    MaterialTheme {
        AppContent(
            state = PREVIEW_STATE.copy(lastAction = "hazir"),
            info = SearchInfo(depth = 7, selDepth = 11, scoreCp = 18,
                nodes = 96_000, timeMs = 240),
            searching = true,
            moveText = "e2e4",
            onMoveTextChange = {}, onPlay = {}, onUndo = {}, onNewGame = {},
            onEngineMove = {}, onStop = {}, onLevel = {},
        )
    }
}
