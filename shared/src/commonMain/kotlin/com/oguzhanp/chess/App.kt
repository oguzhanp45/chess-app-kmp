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
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp

import com.oguzhanp.chess.engine.ChessEngine
import com.oguzhanp.chess.engine.engineVersion

// ============================================================
//  Faz 1.6b -- kopru dogrulama ekrani
// ============================================================
// Bu ekran GECICIDIR ve tasarim degildir. Tek isi motorun Kotlin'den
// gercekten surulebildigini gostermek: hamle oyna, geri al, yeni oyun,
// ve motorun dondurdugu ham durumu gor.
//
// Gercek arayuz Faz 3'te (tahta) ve Faz 5'te (navigasyon) yapilacak;
// oraya gelmeden once tasarim sorulari sorulacak.
//
// Neden ham JSON gosteriyoruz: onu bir Snapshot veri sinifina
// ayristirmak Faz 2'nin isi. Burada ayristirirsak bir hata cikinca
// "kopru mu bozuk, ayristirici mi bozuk" diye tahmin yurutmek zorunda
// kaliriz. Faz 1'in tamami bu tahmini onlemek uzerine kurulu.
//
// Yapi: App() durumu tutar, AppContent() yalnizca aldigini cizer.

@Composable
fun App() {
    MaterialTheme {
        // Motor ornegi ekran omru boyunca yasar.
        val engine = remember { ChessEngine() }

        // C++ tarafindaki nesne kendiliginden yok olmaz; ekran
        // kaldirilirken close() cagirmazsak sizinti olur.
        DisposableEffect(Unit) {
            onDispose { engine.close() }
        }

        var snapshot by remember { mutableStateOf(engine.snapshotJson()) }
        var lastAction by remember { mutableStateOf("hazir") }
        var moveText by remember { mutableStateOf("e2e4") }

        AppContent(
            version = remember { engineVersion() },
            platformName = remember { getPlatform().name },
            moveText = moveText,
            onMoveTextChange = { moveText = it.trim() },
            lastAction = lastAction,
            snapshot = snapshot,
            onPlay = {
                // sanFor HAMLE OYNANMADAN once sorulur -- sonra sorsak
                // pozisyon degismis olurdu ve yanlis cevap alirdik.
                val san = engine.sanFor(moveText)
                val ok = engine.makeMove(moveText)
                lastAction = if (ok) {
                    "makeMove($moveText) = true   SAN: $san"
                } else {
                    "makeMove($moveText) = false  -- legal degil"
                }
                snapshot = engine.snapshotJson()
            },
            onUndo = {
                val ok = engine.undo()
                lastAction = "undo() = $ok"
                snapshot = engine.snapshotJson()
            },
            onNewGame = {
                val ok = engine.newGame("")
                lastAction = "newGame() = $ok"
                snapshot = engine.snapshotJson()
            },
        )
    }
}

@Composable
fun AppContent(
    version: String,
    platformName: String,
    moveText: String,
    onMoveTextChange: (String) -> Unit,
    lastAction: String,
    snapshot: String,
    onPlay: () -> Unit,
    onUndo: () -> Unit,
    onNewGame: () -> Unit,
) {
    Column(
        modifier = Modifier
            .safeContentPadding()
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
    ) {
        Text(version, style = MaterialTheme.typography.titleMedium)
        Text(platformName, style = MaterialTheme.typography.bodySmall)

        Spacer(Modifier.height(20.dp))

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
                modifier = Modifier.weight(1f),
            )
            Button(onClick = onPlay) { Text("Oyna") }
        }

        Spacer(Modifier.height(8.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = onUndo) { Text("Geri al") }
            Button(onClick = onNewGame) { Text("Yeni oyun") }
        }

        Spacer(Modifier.height(20.dp))

        Text("Son islem", style = MaterialTheme.typography.labelMedium)
        Text(
            text = lastAction,
            style = MaterialTheme.typography.bodyMedium,
            fontFamily = FontFamily.Monospace,
        )

        Spacer(Modifier.height(20.dp))

        Text("snapshot (ham JSON)", style = MaterialTheme.typography.labelMedium)
        Text(
            text = snapshot,
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
        )
    }
}

private const val PREVIEW_SNAPSHOT =
    "{\"fen\":\"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2\"," +
        "\"side\":\"w\",\"inCheck\":false,\"status\":\"ongoing\"," +
        "\"legal\":[\"a2a3\",\"a2a4\",\"b2b3\"],\"history\":[\"e4\",\"e5\"]}"

@Preview(showBackground = true)
@Composable
private fun AppContentPreview() {
    MaterialTheme {
        AppContent(
            version = "cpp-chess-engine 1.0 (C++17)",
            platformName = "Android 36",
            moveText = "e2e4",
            onMoveTextChange = {},
            lastAction = "makeMove(e7e5) = true   SAN: e5",
            snapshot = PREVIEW_SNAPSHOT,
            onPlay = {},
            onUndo = {},
            onNewGame = {},
        )
    }
}

@Preview(showBackground = true)
@Composable
private fun AppContentRejectedPreview() {
    MaterialTheme {
        AppContent(
            version = "cpp-chess-engine 1.0 (C++17)",
            platformName = "Android 36",
            moveText = "e2e5",
            onMoveTextChange = {},
            lastAction = "makeMove(e2e5) = false  -- legal degil",
            snapshot = PREVIEW_SNAPSHOT,
            onPlay = {},
            onUndo = {},
            onNewGame = {},
        )
    }
}
