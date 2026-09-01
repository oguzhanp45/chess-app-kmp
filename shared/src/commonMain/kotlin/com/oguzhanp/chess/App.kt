package com.oguzhanp.chess

import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.LinearOutSlowInEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.size
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.scale
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.oguzhanp.chess.data.rememberSettingsRepository
import com.oguzhanp.chess.debug.EngineTestScreen
import com.oguzhanp.chess.onboarding.OnboardingScreen
import com.oguzhanp.chess.resources.Res
import com.oguzhanp.chess.resources.chess_logo
import com.oguzhanp.chess.theme.ChessTheme
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.jetbrains.compose.resources.painterResource

// ============================================================
//  App -- uygulamanin kok bileseni
// ============================================================
// Iki platformun giris noktasi da burayi cagiriyor:
//   Android -> MainActivity
//   iOS     -> MainViewController
//
// Tek isi: temayi kurmak ve HANGI ekranin gosterilecegine karar vermek.
// Ekranlarin kendi ici bu dosyayi ilgilendirmiyor.
//
// NAVIGATION YOK, bilerek. Su an iki hedef var ve onboarding bittikten
// sonra ona geri donulmuyor -- yani bir geri yigini gerekmiyor.
// Navigation Faz 5'te, bes sekme ve her sekmenin kendi yigini
// geldiginde eklenecek.

@Composable
fun App() {
    ChessTheme {
        val settings = rememberSettingsRepository()
        val scope = rememberCoroutineScope()

        // UC durum var, iki degil:
        //   null  -> diskten henuz okunmadi
        //   false -> tanitim gosterilecek
        //   true  -> uygulamaya girilecek
        //
        // null halini atlarsak once ana ekran cizilir, 50 ms sonra deger
        // gelir ve ekran tanitima sicrar. Kullanici bunu TITREME olarak
        // gorur. RN surumunde tahtada yasadigimiz sorunun ayni sinifi:
        // durum gec geldigi icin ekran iki kez ciziliyor.
        var onboarded by remember { mutableStateOf<Boolean?>(null) }

        // Acilis animasyonu bitti mi. Yapay bir gecikme DEGIL: uygulama
        // iki sart birden saglaninca geciyor -- veri geldi VE animasyon
        // bitti. Veri yavas gelirse fazladan beklemiyoruz, hizli gelirse
        // animasyon yarida kesilmiyor.
        var splashFinished by remember { mutableStateOf(false) }

        LaunchedEffect(settings) {
            settings.isOnboarded.collect { onboarded = it }
        }

        val ready = onboarded != null && splashFinished

        when {
            !ready -> SplashScreen(onFinished = { splashFinished = true })

            onboarded == false -> OnboardingScreen(
                onFinish = { scope.launch { settings.setOnboarded(true) } },
            )

            else -> EngineTestScreen()
        }
    }
}

// Acilis animasyonu olculeri. Toplam gorunur sure ~700 ms.
private const val SPLASH_FADE_MS = 500
private const val SPLASH_HOLD_MS = 200L
private val SplashLogoSize = 112.dp

/**
 * Acilis ekrani: logo belirip hafifce buyur, sonra devam edilir.
 *
 * NOT (Faz 8): Android 12'den beri SISTEMIN kendi splash'i da var --
 * uygulama simgesi Compose baslamadan once gosteriliyor. Yani su an iki
 * acilis ekrani ust uste biniyor. Ikisini hizalamak cila isi.
 */
@Composable
private fun SplashScreen(
    onFinished: () -> Unit,
    modifier: Modifier = Modifier,
) {
    // Animatable: hedefe kadar olan gecisi elle surdugumuz deger.
    // animate*AsState'ten farki, bitmesini BEKLEYEBILMEMIZ.
    val alpha = remember { Animatable(0f) }
    val scale = remember { Animatable(0.86f) }

    LaunchedEffect(Unit) {
        // Buyume ile solma ayni anda kossun diye biri ayri baslatiliyor.
        launch {
            scale.animateTo(
                targetValue = 1f,
                animationSpec = tween(SPLASH_FADE_MS, easing = FastOutSlowInEasing),
            )
        }
        alpha.animateTo(
            targetValue = 1f,
            animationSpec = tween(SPLASH_FADE_MS, easing = LinearOutSlowInEasing),
        )
        delay(SPLASH_HOLD_MS)   // logo bir an dursun, aninda kaybolmasin
        onFinished()
    }

    Box(
        modifier = modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background),
        contentAlignment = Alignment.Center,
    ) {
        Icon(
            painter = painterResource(Res.drawable.chess_logo),
            contentDescription = null,
            tint = MaterialTheme.colorScheme.primary,
            modifier = Modifier
                .size(SplashLogoSize)
                .alpha(alpha.value)
                .scale(scale.value),
        )
    }
}

@Preview
@Composable
private fun SplashScreenPreview() {
    ChessTheme {
        SplashScreen(onFinished = {})
    }
}
