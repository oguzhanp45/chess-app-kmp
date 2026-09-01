package com.oguzhanp.chess.theme

import androidx.compose.ui.graphics.Color

// ============================================================
//  Renk paleti -- tek kaynak
// ============================================================
// Mimari kural 6: renk ve olcu sabitleri tek dosyada toplanir.
// Ekran kodunda ASLA duz renk yazilmaz; her sey MaterialTheme
// uzerinden okunur. Boylece tema degistiginde tek dosya degisiyor.
//
// Yon: KLASIK VE LUKS.
//   - notr zemin: soguk gri degil, sicak fildisi / kahve
//   - vurgu: altin-bronz
//   - metin: saf siyah/beyaz degil, kirik tonlar (goz yormasin)

// --- Acik tema ---
internal val LightPrimary          = Color(0xFF7A5C33)   // derin bronz
internal val LightOnPrimary        = Color(0xFFFFFFFF)
internal val LightPrimaryContainer = Color(0xFFE8DCC6)   // altin fisilti
internal val LightOnPrimaryContainer = Color(0xFF2B2118)
internal val LightSecondary        = Color(0xFF5C5347)
internal val LightOnSecondary      = Color(0xFFFFFFFF)
internal val LightBackground       = Color(0xFFF7F2E9)   // fildisi
internal val LightOnBackground     = Color(0xFF241D14)
internal val LightSurface          = Color(0xFFF7F2E9)
internal val LightOnSurface        = Color(0xFF241D14)
internal val LightSurfaceVariant   = Color(0xFFE7DFD1)
internal val LightOnSurfaceVariant = Color(0xFF4E463A)   // ikincil metin
internal val LightOutline          = Color(0xFF8A8070)

// --- Koyu tema ---
// Yumusatildi: zemin saf siyah degil sicak komur, metin parlak beyaz
// degil kirik fildisi, altin ise dusuk doygunlukta. Koyu temada yuksek
// kontrast ve yuksek doygunluk gozu yorar -- amac okunakli ama sakin
// bir yuzey.
//
// Ayar yapmak istersen uc kaldirac var:
//   DarkBackground     daha acik = daha yumusak (0xFF16120E -> 0xFF221E1A)
//   DarkOnBackground   daha koyu = daha az parlama
//   DarkPrimary        daha gri = daha sakin altin
internal val DarkPrimary          = Color(0xFFCBB08A)   // sakin altin, dusuk doygunluk
internal val DarkOnPrimary        = Color(0xFF35291A)
internal val DarkPrimaryContainer = Color(0xFF453A2C)   // yumusak kahve
internal val DarkOnPrimaryContainer = Color(0xFFEFE3D0)
internal val DarkSecondary        = Color(0xFFC4BCAE)
internal val DarkOnSecondary      = Color(0xFF33302A)
internal val DarkBackground       = Color(0xFF1B1815)   // sicak komur, siyah degil
internal val DarkOnBackground     = Color(0xFFE4DCD0)   // kirik fildisi
internal val DarkSurface          = Color(0xFF1B1815)
internal val DarkOnSurface        = Color(0xFFE4DCD0)
internal val DarkSurfaceVariant   = Color(0xFF37322B)
internal val DarkOnSurfaceVariant = Color(0xFFC0B8AB)   // ikincil metin
internal val DarkOutline          = Color(0xFF857D71)
