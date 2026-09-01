package com.oguzhanp.chess.data

import androidx.compose.runtime.Composable

// ============================================================
//  SettingsRepository'yi kurma noktasi
// ============================================================
// Android'de DataStore bir Context ister, iOS'ta istemez. Bu fark
// yalnizca KURULUM aninda var; kurulduktan sonra iki platform da ayni
// SettingsRepository'yi kullaniyor.
//
// Bu yuzden expect/actual'i repository'ye degil, onu KURAN fonksiyona
// koyduk. Uygulama kodu (App.kt) tek satir yaziyor ve platformu
// hic bilmiyor.
//
// Neden @Composable: Android tarafinda Context'e ulasmanin en temiz
// yolu LocalContext. Boylece MainActivity'ye dokunmaya gerek kalmiyor.

@Composable
internal expect fun rememberSettingsRepository(): SettingsRepository
