package com.oguzhanp.chess.onboarding

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.animateDpAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeContentPadding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.oguzhanp.chess.theme.ChessTheme
import kotlinx.coroutines.launch

// ============================================================
//  Onboarding -- uygulamayi ilk acista gosterilen 3 ekran
// ============================================================
// Bu dosya commonMain'de: icinde platforma ozel hicbir sey yok.
//
// Burada DataStore ya da navigation YOK. Ekran "bittim" dedigini
// onFinish ile disariya haber veriyor; sonrasinda ne olacagini
// cagiran taraf biliyor. Boylece ekran tek basina onizlenebiliyor.
//
// Renkler dogrudan yazilmiyor, hepsi MaterialTheme'den okunuyor.
// Bu yuzden acik/koyu mod ayri bir is gerektirmiyor -- tema degisince
// ekran kendiliginde degisiyor.

// --- Olculer: tek yerde, mimari kural 6 ---
private val TextMaxWidth = 340.dp
private val SkipBarHeight = 56.dp
private val DotSize = 8.dp
private val DotActiveWidth = 28.dp

/** Sayfanin ustunde hangi animasyonun cizilecegi. */
private enum class OnboardingVisual { LEVELS, RATING, OFFLINE }

/** Tek bir onboarding sayfasinin icerigi. */
private data class OnboardingPage(
    val visual: OnboardingVisual,
    val title: String,
    val description: String,
)

// Metinler simdilik burada. Faz 6'da dil destegi gelince sozlukten
// gelecek (mimari kural 7) -- veriyi ayri tuttugumuz icin o gecis
// yalnizca bu listeyi degistirmek olacak.
private val onboardingPages = listOf(
    OnboardingPage(
        visual = OnboardingVisual.LEVELS,
        title = "Satranca hos geldin",
        description = "Guclu bir satranc motoruna karsi oyna. " +
            "Alti farkli seviye, acemiden tam guce.",
    ),
    OnboardingPage(
        visual = OnboardingVisual.RATING,
        title = "Kendini gelistir",
        description = "Bulmacalar, dersler ve mac sonu analiziyle " +
            "nerede hata yaptigini gor.",
    ),
    OnboardingPage(
        visual = OnboardingVisual.OFFLINE,
        title = "Her yerde yaninda",
        description = "Internet gerekmez. Oyunlarin ve ilerlemen " +
            "cihazinda saklanir.",
    ),
)

@Composable
fun OnboardingScreen(
    onFinish: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val pagerState = rememberPagerState(pageCount = { onboardingPages.size })

    // animateScrollToPage bir suspend fonksiyon; tiklamada baslatilacak
    // bir coroutine gerekiyor. rememberCoroutineScope bu ekrana bagli bir
    // kapsam veriyor -- ekran kaybolunca isler iptal olur.
    val scope = rememberCoroutineScope()

    Column(
        modifier = modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .safeContentPadding(),
    ) {
        // "Gec" barinin yuksekligi SABIT: son sayfada tus kaybolunca
        // altindaki her sey yukari kaymasin. Gorunurlugu degisen bir
        // ogeye yer ayirmak, duzenin zipladigini gormemek demek.
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(SkipBarHeight)
                .padding(horizontal = 12.dp),
            contentAlignment = Alignment.CenterEnd,
        ) {
            val isLastPage = pagerState.currentPage == onboardingPages.size - 1
            if (!isLastPage) {
                TextButton(onClick = onFinish) {
                    Text(
                        text = "Gec",
                        style = MaterialTheme.typography.labelLarge,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                    )
                }
            }
        }

        // weight(1f): kalan butun yuksekligi pager alsin, alt kisim
        // sabit kalsin. Tuslar pager'in ICINDE olsaydi sayfayla kayardi.
        HorizontalPager(
            state = pagerState,
            modifier = Modifier.weight(1f),
        ) { page ->
            OnboardingPageContent(page = onboardingPages[page])
        }

        PageIndicator(
            currentPage = pagerState.currentPage,
            pageCount = onboardingPages.size,
            modifier = Modifier.padding(bottom = 8.dp),
        )

        OnboardingControls(
            currentPage = pagerState.currentPage,
            pageCount = onboardingPages.size,
            onBack = {
                scope.launch { pagerState.animateScrollToPage(pagerState.currentPage - 1) }
            },
            onNext = {
                scope.launch { pagerState.animateScrollToPage(pagerState.currentPage + 1) }
            },
            onFinish = onFinish,
        )
    }
}

@Composable
private fun OnboardingPageContent(
    page: OnboardingPage,
    modifier: Modifier = Modifier,
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(horizontal = 32.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        // Sayfaya ozel animasyon. Ucu de ayni olcude bir kutuya
        // sigiyor (VisualSize), boylece sayfalar arasinda kayarken
        // baslik ve metin ayni yerde duruyor.
        Box(
            modifier = Modifier.size(VisualSize),
            contentAlignment = Alignment.Center,
        ) {
            when (page.visual) {
                OnboardingVisual.LEVELS -> LevelsVisual()
                OnboardingVisual.RATING -> RatingVisual()
                OnboardingVisual.OFFLINE -> OfflineVisual()
            }
        }

        Spacer(Modifier.height(40.dp))

        Text(
            text = page.title,
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.SemiBold,
            color = MaterialTheme.colorScheme.onBackground,
            textAlign = TextAlign.Center,
        )

        Spacer(Modifier.height(16.dp))

        Text(
            text = page.description,
            style = MaterialTheme.typography.bodyLarge,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = TextAlign.Center,
            lineHeight = 24.sp,
            // Genis ekranda satirlar cok uzamasin: okunabilirlik icin
            // satir uzunlugu sinirlanir. Tablette fark ediliyor.
            modifier = Modifier.widthIn(max = TextMaxWidth),
        )
    }
}

/** Kacinci sayfada oldugumuzu gosteren noktalar. Aktif olan genisler. */
@Composable
private fun PageIndicator(
    currentPage: Int,
    pageCount: Int,
    modifier: Modifier = Modifier,
) {
    Row(
        modifier = modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.Center,
        verticalAlignment = Alignment.CenterVertically,
    ) {
        repeat(pageCount) { index ->
            val selected = index == currentPage

            // animate*AsState: hedef deger degisince Compose aradaki
            // gecisi kendisi cizer. Elle animasyon yazmaya gerek yok.
            val width by animateDpAsState(
                targetValue = if (selected) DotActiveWidth else DotSize,
                label = "dotWidth",
            )
            val color by animateColorAsState(
                targetValue = if (selected) {
                    MaterialTheme.colorScheme.primary
                } else {
                    MaterialTheme.colorScheme.outline.copy(alpha = 0.4f)
                },
                label = "dotColor",
            )

            Box(
                modifier = Modifier
                    .padding(horizontal = 4.dp)
                    .size(width = width, height = DotSize)
                    .background(color = color, shape = RoundedCornerShape(percent = 50)),
            )
        }
    }
}

@Composable
private fun OnboardingControls(
    currentPage: Int,
    pageCount: Int,
    onBack: () -> Unit,
    onNext: () -> Unit,
    onFinish: () -> Unit,
    modifier: Modifier = Modifier,
) {
    val isFirst = currentPage == 0
    val isLast = currentPage == pageCount - 1

    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 24.dp, vertical = 20.dp),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        // Ilk sayfada geri tusu yok. Spacer(weight) sagdaki tusu her
        // durumda saga itiyor, yani duzen tek dala bagli degil.
        if (!isFirst) {
            TextButton(onClick = onBack) {
                Text("Geri", style = MaterialTheme.typography.labelLarge)
            }
        }

        Spacer(Modifier.weight(1f))

        Button(
            onClick = if (isLast) onFinish else onNext,
            shape = RoundedCornerShape(12.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary,
                contentColor = MaterialTheme.colorScheme.onPrimary,
            ),
            contentPadding = PaddingValues(horizontal = 28.dp, vertical = 14.dp),
        ) {
            Text(
                text = if (isLast) "Basla" else "Devam",
                style = MaterialTheme.typography.labelLarge,
                fontWeight = FontWeight.SemiBold,
            )
        }
    }
}

// ------------------------------------------------------------
//  Onizlemeler -- acik ve koyu, ikisi de
// ------------------------------------------------------------

@Preview
@Composable
private fun OnboardingScreenLightPreview() {
    ChessTheme(darkTheme = false) {
        Surface { OnboardingScreen(onFinish = {}) }
    }
}

@Preview
@Composable
private fun OnboardingScreenDarkPreview() {
    ChessTheme(darkTheme = true) {
        Surface { OnboardingScreen(onFinish = {}) }
    }
}

// Tuslar durumsuz oldugu icin son sayfanin halini pager'a hic
// dokunmadan onizleyebiliyoruz.
@Preview
@Composable
private fun OnboardingControlsLastPagePreview() {
    ChessTheme(darkTheme = false) {
        Surface {
            OnboardingControls(
                currentPage = 2,
                pageCount = 3,
                onBack = {},
                onNext = {},
                onFinish = {},
            )
        }
    }
}
