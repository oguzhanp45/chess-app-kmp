package com.oguzhanp.chess

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeContentPadding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp

import com.oguzhanp.chess.engine.engineSelfTest
import com.oguzhanp.chess.engine.engineVersion

// ============================================================
//  Faz 1.5 -- kopru dogrulama ekrani
// ============================================================
// Bu ekran GECICIDIR ve tasarim degildir. Tek isi motorun Kotlin'den
// okunabildigini gostermek. Gercek arayuz Faz 3'te (tahta) ve Faz 5'te
// (navigasyon) yapilacak; oraya gelmeden once tasarim sorulari sorulacak.
//
// Dikkat: bu dosya commonMain'de, yani JNI'i de cinterop'u da bilmiyor.
// engineVersion() cagrisinin altinda Android'de JNI, iOS'ta (ileride)
// cinterop var; ekran kodu ikisini de gormuyor. Mimari kural 5.
//
// Yapi: App() durumu toplar, AppContent() yalnizca aldigi parametreleri
// cizer. Bu ayrim sayesinde AppContent kopruye hic dokunmadan
// onizlenebiliyor ve test edilebiliyor.

@Composable
fun App() {
    MaterialTheme {
        // remember: yeniden cizimlerde kopruye tekrar gidilmesin.
        val version = remember { engineVersion() }
        val legalMoves = remember { engineSelfTest() }
        val platformName = remember { getPlatform().name }

        AppContent(
            version = version,
            legalMoves = legalMoves,
            platformName = platformName,
        )
    }
}

@Composable
fun AppContent(
    version: String,
    legalMoves: Int,
    platformName: String,
) {
    Column(
        modifier = Modifier
            .safeContentPadding()
            .fillMaxSize()
            .padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center,
    ) {
        Text(
            text = version,
            style = MaterialTheme.typography.titleMedium,
            textAlign = TextAlign.Center,
        )

        Text(
            text = if (legalMoves == 20) {
                "Oz test: $legalMoves legal hamle -- kopru calisiyor"
            } else {
                "Oz test BASARISIZ: $legalMoves (20 bekleniyordu)"
            },
            style = MaterialTheme.typography.bodyMedium,
            textAlign = TextAlign.Center,
            modifier = Modifier.padding(top = 12.dp),
        )

        Text(
            text = platformName,
            style = MaterialTheme.typography.bodySmall,
            modifier = Modifier.padding(top = 24.dp),
        )
    }
}

@Preview(showBackground = true)
@Composable
private fun AppContentPreview() {
    MaterialTheme {
        AppContent(
            version = "cpp-chess-engine 1.0 (C++17)",
            legalMoves = 20,
            platformName = "Android 36",
        )
    }
}

// Oz test basarisiz oldugunda ekranin nasil gorundugunu de onizleyelim --
// hata halini gormeden dogru yazdigimizdan emin olamayiz.
@Preview(showBackground = true)
@Composable
private fun AppContentFailurePreview() {
    MaterialTheme {
        AppContent(
            version = "cpp-chess-engine 1.0 (C++17)",
            legalMoves = -1,
            platformName = "Android 36",
        )
    }
}
