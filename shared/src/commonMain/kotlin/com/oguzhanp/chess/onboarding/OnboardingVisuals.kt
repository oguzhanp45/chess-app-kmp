package com.oguzhanp.chess.onboarding

import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
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
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.util.lerp
import kotlin.math.roundToInt

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

// Sayacin okunabilmesi icin adim adim artiyor. Her karede degisseydi
// (saniyede 60 kez) hicbir sayi okunamazdi -- bulanik bir titresim olurdu.
private const val RATING_STEP = 25

// Cizginin kirilma noktalari: x = 0..1, y = yukseklik orani (0 = alt).
// Duz bir cizgi yerine ufak inisler var; gelisim hicbir zaman duz
// degildir ve goze daha inandirici gelir.
private val progressPoints = listOf(
    0.04f to 0.12f,
    0.20f to 0.30f,
    0.36f to 0.24f,
    0.52f to 0.48f,
    0.68f to 0.60f,
    0.84f to 0.74f,
    0.96f to 0.94f,
)

/**
 * Sayfa 2 -- gelisim.
 *
 * Yukselen bir cizgi ciziliyor, ucundaki nokta ilerliyor, altinda puan
 * artiyor. Bilerek CIZGI: birinci sayfa cubuk kullaniyor, ayni gorsel
 * dili iki kez kullanirsak sayfalar birbirinden ayrilmaz.
 */
@Composable
internal fun ProgressVisual(modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "progress")

    val progress by transition.animateFloat(
        initialValue = 0f,
        targetValue = 1f,
        animationSpec = infiniteRepeatable(
            // 5 saniye: cizgi rahat izlenebilsin, sayi okunabilsin.
            animation = tween(durationMillis = 5000, easing = LinearEasing),
            repeatMode = RepeatMode.Restart,
        ),
        label = "progressValue",
    )

    val rawRating = RATING_START + (RATING_END - RATING_START) * progress
    val rating = (rawRating / RATING_STEP).roundToInt() * RATING_STEP

    val lineColor = MaterialTheme.colorScheme.primary
    val trackColor = MaterialTheme.colorScheme.primaryContainer

    Column(
        modifier = modifier.size(VisualSize),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Canvas(
            modifier = Modifier
                .fillMaxWidth()
                .height(112.dp)
                .padding(horizontal = 8.dp),
        ) {
            fun px(point: Pair<Float, Float>) = Offset(
                x = point.first * size.width,
                y = (1f - point.second) * size.height,
            )

            // Soluk iz: cizginin gidecegi yol bastan gorunuyor, boylece
            // animasyon bos bir alanda baslamiyor.
            val track = Path()
            track.moveTo(px(progressPoints.first()).x, px(progressPoints.first()).y)
            progressPoints.drop(1).forEach { track.lineTo(px(it).x, px(it).y) }
            drawPath(
                path = track,
                color = trackColor,
                style = Stroke(width = 3.dp.toPx(), cap = StrokeCap.Round, join = StrokeJoin.Round),
            )

            // Cizilen kisim: ilerlemeye gore son noktayi ara degerle bul.
            val segments = progressPoints.size - 1
            val exact = progress * segments
            val index = exact.toInt().coerceAtMost(segments - 1)
            val t = exact - index

            val from = progressPoints[index]
            val to = progressPoints[index + 1]
            val head = Offset(
                x = lerp(px(from).x, px(to).x, t),
                y = lerp(px(from).y, px(to).y, t),
            )

            val drawn = Path()
            drawn.moveTo(px(progressPoints.first()).x, px(progressPoints.first()).y)
            for (i in 1..index) {
                drawn.lineTo(px(progressPoints[i]).x, px(progressPoints[i]).y)
            }
            drawn.lineTo(head.x, head.y)
            drawPath(
                path = drawn,
                color = lineColor,
                style = Stroke(width = 6.dp.toPx(), cap = StrokeCap.Round, join = StrokeJoin.Round),
            )

            drawCircle(color = lineColor, radius = 7.dp.toPx(), center = head)
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

private const val BOARD_SIZE = 8
private val BoardEdge = 168.dp

/**
 * Sayfa 3 -- her sey cihazinda.
 *
 * Kucuk bir tahta capraz bir dalga halinde beliriyor ve kayboluyor.
 * Ucuncu gorsel dili: cubuk degil, cizgi degil, IZGARA. Uc sayfa uc
 * ayri sekil ailesi kullaniyor, boylece kaydirirken degisim hissediliyor.
 */
@Composable
internal fun BoardVisual(modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "board")

    val progress by transition.animateFloat(
        initialValue = 0f,
        targetValue = 1f,
        animationSpec = infiniteRepeatable(
            animation = tween(durationMillis = 3000, easing = LinearEasing),
            // Reverse: tahta kuruluyor, sonra ayni yumusaklikta cozuluyor.
            // Restart olsaydi her turda sert bir sicrama olurdu.
            repeatMode = RepeatMode.Reverse,
        ),
        label = "boardProgress",
    )

    val lightSquare = MaterialTheme.colorScheme.primaryContainer
    val darkSquare = MaterialTheme.colorScheme.primary
    val cell = BoardEdge / BOARD_SIZE

    Box(
        modifier = modifier.size(VisualSize),
        contentAlignment = Alignment.Center,
    ) {
        Column(
            modifier = Modifier
                .size(BoardEdge)
                .clip(RoundedCornerShape(10.dp)),
        ) {
            repeat(BOARD_SIZE) { row ->
                Row {
                    repeat(BOARD_SIZE) { column ->
                        // Capraz dalga: sol ustten sag alta dogru aciliyor.
                        val wave = (row + column).toFloat() / (2 * BOARD_SIZE - 2)
                        // 1.35 carpani: son kare de tam gorunur olsun diye
                        // ilerlemeye bir miktar pay birakiyoruz.
                        val alpha = ((progress * 1.35f - wave) * 3f).coerceIn(0f, 1f)

                        Box(
                            modifier = Modifier
                                .size(cell)
                                .background(
                                    if ((row + column) % 2 == 0) lightSquare else darkSquare
                                )
                                .alpha(alpha),
                        )
                    }
                }
            }
        }
    }
}
