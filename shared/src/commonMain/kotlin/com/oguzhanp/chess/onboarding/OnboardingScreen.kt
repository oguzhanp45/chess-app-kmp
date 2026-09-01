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
import com.oguzhanp.chess.resources.Res
import com.oguzhanp.chess.resources.onboarding_back
import com.oguzhanp.chess.resources.onboarding_body_engine
import com.oguzhanp.chess.resources.onboarding_body_offline
import com.oguzhanp.chess.resources.onboarding_body_progress
import com.oguzhanp.chess.resources.onboarding_next
import com.oguzhanp.chess.resources.onboarding_skip
import com.oguzhanp.chess.resources.onboarding_start
import com.oguzhanp.chess.resources.onboarding_title_engine
import com.oguzhanp.chess.resources.onboarding_title_offline
import com.oguzhanp.chess.resources.onboarding_title_progress
import com.oguzhanp.chess.theme.ChessTheme
import com.oguzhanp.chess.theme.Corners
import com.oguzhanp.chess.theme.Spacing
import org.jetbrains.compose.resources.StringResource
import org.jetbrains.compose.resources.stringResource
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
private enum class OnboardingVisual { LEVELS, PROGRESS, BOARD }

/** Tek bir onboarding sayfasinin icerigi. */
private data class OnboardingPage(
    val visual: OnboardingVisual,
    val title: StringResource,
    val description: StringResource,
)

// Metinler artik sozlukten geliyor (mimari kural 7). Compose
// Resources cihazin diline gore values/ ya da values-tr/ dosyasini
// seciyor -- ek kod yazmiyoruz.
private val onboardingPages = listOf(
    OnboardingPage(
        visual = OnboardingVisual.LEVELS,
        title = Res.string.onboarding_title_engine,
        description = Res.string.onboarding_body_engine,
    ),
    OnboardingPage(
        visual = OnboardingVisual.PROGRESS,
        title = Res.string.onboarding_title_progress,
        description = Res.string.onboarding_body_progress,
    ),
    OnboardingPage(
        visual = OnboardingVisual.BOARD,
        title = Res.string.onboarding_title_offline,
        description = Res.string.onboarding_body_offline,
    ))

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
                .padding(horizontal = Spacing.sm + Spacing.xs),
            contentAlignment = Alignment.CenterEnd,
        ) {
            val isLastPage = pagerState.currentPage == onboardingPages.size - 1
            if (!isLastPage) {
                TextButton(onClick = onFinish) {
                    Text(
                        text = stringResource(Res.string.onboarding_skip),
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
            .padding(horizontal = Spacing.xl),
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
                OnboardingVisual.PROGRESS -> ProgressVisual()
                OnboardingVisual.BOARD -> BoardVisual()
            }
        }

        Spacer(Modifier.height(Spacing.xxl))

        Text(
            text = stringResource(page.title),
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.SemiBold,
            color = MaterialTheme.colorScheme.onBackground,
            textAlign = TextAlign.Center,
        )

        Spacer(Modifier.height(Spacing.md))

        Text(
            text = stringResource(page.description),
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
                    .padding(horizontal = Spacing.xs)
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
            .padding(horizontal = Spacing.lg, vertical = Spacing.lg),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        // Ilk sayfada geri tusu yok. Spacer(weight) sagdaki tusu her
        // durumda saga itiyor, yani duzen tek dala bagli degil.
        if (!isFirst) {
            TextButton(onClick = onBack) {
                Text(stringResource(Res.string.onboarding_back), style = MaterialTheme.typography.labelLarge)
            }
        }

        Spacer(Modifier.weight(1f))

        Button(
            onClick = if (isLast) onFinish else onNext,
            shape = RoundedCornerShape(Corners.md),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary,
                contentColor = MaterialTheme.colorScheme.onPrimary,
            ),
            contentPadding = PaddingValues(horizontal = Spacing.lg + Spacing.xs, vertical = Spacing.md - Spacing.xs),
        ) {
            Text(
                text = stringResource(
                    if (isLast) Res.string.onboarding_start else Res.string.onboarding_next
                ),
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
