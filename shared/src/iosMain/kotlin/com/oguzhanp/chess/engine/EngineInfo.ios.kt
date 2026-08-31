package com.oguzhanp.chess.engine

// iOS tarafi henuz motora bagli DEGIL. Motoru iOS icin derleyip
// cinterop ile baglamak Faz 1.7'nin isi.
//
// Bu yer tutucu bilerek duruyor: iosArm64 hedefinin derlenebilir
// kalmasini sagliyor, boylece GitHub Actions'taki macOS kosucusu
// her commit'te iOS tarafinin bozulmadigini dogrulayabiliyor.

actual fun engineVersion(): String = "iOS: motor henuz baglanmadi"

actual fun engineSelfTest(): Int = -1
