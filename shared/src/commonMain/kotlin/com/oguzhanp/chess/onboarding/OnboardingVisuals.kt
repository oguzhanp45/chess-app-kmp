package com.oguzhanp.chess.onboarding

import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.scale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.oguzhanp.chess.resources.Res
import com.oguzhanp.chess.resources.chess_logo
import kotlin.math.roundToInt
import org.jetbrains.compose.resources.painterResource

// ============================================================
//  Onboarding gorselleri
// ============================================================
// Her sayfanin kendi animasyonu var; ucu de yalnizca Compose
// ilkelleriyle (kutu, metin, animasyon) cizildi. Disaridan gorsel
// dosyasi gerekmiyor -- yani lisans ve boyut derdi yok.
//
// Hepsi AYNI olcude bir kutuya siginiyor (VisualSize). Boylece
// sayfalar arasinda kayarken duzen zipllamiyor.
//
// NOT: bu gorseller uygulamanin icerigi tamamlandiginda gozden
// gecirilecek (yol haritasi -> plan disi eklenenler).

internal val VisualSize = 200.dp

private val LevelBarWidth = 14.dp
private val LevelBarMaxHeight = 96.dp
private val LevelBarGap = 8.dp

/** Motorun alti seviyesi. Faz 6'da sozluge tasinacak. */
private val levelNames = listOf(
    "Acemi", "Baslangic", "Orta", "Iyi", "Zor", "Tam guc",
)

/**
 * Sayfa 1 -- motorun gucu.
 *
 * Alti cubuk sirayla doluyor ve altindaki seviye adi degisiyor.
 * Motorun gercekten alti isimli kademesi oldugu icin bu sadece sus
 * degil, bilgi veriyor.
 */
@Composable
internal fun LevelsVisual(modifier: Modifier = Modifier) {
    // rememberInfiniteTransition: durmadan tekrarlayan animasyonlarin
    // ortak saati. Tek tek zamanlayici kurmaktan iyidir.
    val transition = rememberInfiniteTransition(label = "levels")

    // 0f -> 6f arasinda suruklenen bir imlec. Tam sayi kismi hangi
    // seviyede oldugumuzu, ondalik kismi gecisi veriyor.
    val progress by transition.animateFloat(
        initialValue = 0f,
        targetValue = levelNames.size.toFloat(),
        animationSpec = infiniteRepeatable(
            animation = tween(durationMillis = 4200, easing = LinearEasing),
            repeatMode = RepeatMode.Restart,
        ),
        label = "levelProgress",
    )

    val activeIndex = progress.toInt().coerceIn(0, levelNames.lastIndex)

    Column(
        modifier = modifier.size(VisualSize),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Row(
            verticalAlignment = Alignment.Bottom,
            horizontalArrangement = Arrangement.spacedBy(LevelBarGap),
            modifier = Modifier.height(LevelBarMaxHeight),
        ) {
            levelNames.forEachIndexed { index, _ ->
                val filled = index <= activeIndex
                // Cubuk boylari kademeli artiyor: en zayif kisa, en guclu uzun.
                val heightFraction = (index + 1).toFloat() / levelNames.size
                Box(
                    modifier = Modifier
                        .width(LevelBarWidth)
                        .height(LevelBarMaxHeight * heightFraction)
                        .background(
                            color = if (filled) {
                                MaterialTheme.colorScheme.primary
                            } else {
                                MaterialTheme.colorScheme.primaryContainer
                            },
                            shape = RoundedCornerShape(topStart = 6.dp, topEnd = 6.dp),
                        ),
                )
            }
        }

        Box(
            modifier = Modifier.height(40.dp).fillMaxWidth(),
            contentAlignment = Alignment.Center,
        ) {
            Text(
                text = levelNames[activeIndex],
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.primary,
            )
        }
    }
}

private const val RATING_START = 800
private const val RATING_END = 1450

/**
 * Sayfa 2 -- gelisim.
 *
 * Puan sayaci artiyor, arkasindaki merdiven cubuklari sirayla doluyor.
 */
@Composable
internal fun RatingVisual(modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "rating")

    val progress by transition.animateFloat(
        initialValue = 0f,
        targetValue = 1f,
        animationSpec = infiniteRepeatable(
            animation = tween(durationMillis = 2600, easing = LinearEasing),
            repeatMode = RepeatMode.Restart,
        ),
        label = "ratingProgress",
    )

    val rating = (RATING_START + (RATING_END - RATING_START) * progress).roundToInt()
    val barCount = 7

    Column(
        modifier = modifier.size(VisualSize),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Row(
            verticalAlignment = Alignment.Bottom,
            horizontalArrangement = Arrangement.spacedBy(6.dp),
            modifier = Modifier.height(LevelBarMaxHeight),
        ) {
            repeat(barCount) { index ->
                val threshold = index.toFloat() / barCount
                val filled = progress > threshold
                Box(
                    modifier = Modifier
                        .width(12.dp)
                        .height(LevelBarMaxHeight * ((index + 1).toFloat() / barCount))
                        .background(
                            color = if (filled) {
                                MaterialTheme.colorScheme.primary
                            } else {
                                MaterialTheme.colorScheme.primaryContainer
                            },
                            shape = RoundedCornerShape(topStart = 5.dp, topEnd = 5.dp),
                        ),
                )
            }
        }

        Box(
            modifier = Modifier.height(40.dp).fillMaxWidth(),
            contentAlignment = Alignment.Center,
        ) {
            Text(
                text = "$rating puan",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold,
                color = MaterialTheme.colorScheme.primary,
            )
        }
    }
}

/**
 * Sayfa 3 -- cevrimdisi.
 *
 * Logo bir cihaz cercevesinin icinde, etrafinda yavasca nefes alan bir
 * halka: "her sey cihazinda, yaninda".
 */
@Composable
internal fun OfflineVisual(modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "offline")

    val pulse by transition.animateFloat(
        initialValue = 0f,
        targetValue = 1f,
        animationSpec = infiniteRepeatable(
            animation = tween(durationMillis = 2000, easing = LinearEasing),
            repeatMode = RepeatMode.Reverse,
        ),
        label = "pulse",
    )

    Box(
        modifier = modifier.size(VisualSize),
        contentAlignment = Alignment.Center,
    ) {
        // Nefes alan halka: buyurken soluyor.
        Box(
            modifier = Modifier
                .size(VisualSize)
                .scale(0.80f + pulse * 0.20f)
                .alpha(0.35f - pulse * 0.25f)
                .background(
                    color = MaterialTheme.colorScheme.primary,
                    shape = CircleShape,
                ),
        )

        // Cihaz cercevesi.
        Box(
            modifier = Modifier
                .size(width = 108.dp, height = 148.dp)
                .background(
                    color = MaterialTheme.colorScheme.primaryContainer,
                    shape = RoundedCornerShape(20.dp),
                )
                .padding(16.dp),
            contentAlignment = Alignment.Center,
        ) {
            Icon(
                painter = painterResource(Res.drawable.chess_logo),
                contentDescription = null,
                tint = MaterialTheme.colorScheme.primary,
                modifier = Modifier.size(64.dp),
            )
        }
    }
}
