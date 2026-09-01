package com.oguzhanp.chess.data

import android.content.Context
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.platform.LocalContext

// Surec basina TEK ornek. Ayni dosya icin ikinci bir DataStore
// olusturmak calisma aninda hata verir; Compose ise bir composable'i
// terk edip tekrar girebildigi icin remember tek basina yetmiyor.
private var instance: SettingsRepository? = null

@Composable
internal actual fun rememberSettingsRepository(): SettingsRepository {
    // applicationContext: Activity'ye tutunmuyoruz, yoksa ekran donunce
    // yok olmus bir Activity'yi elimizde tutmus oluruz (bellek sizintisi).
    val context = LocalContext.current.applicationContext

    return remember {
        instance ?: SettingsRepository(
            createSettingsDataStore { settingsFilePath(context) }
        ).also { instance = it }
    }
}

/** Uygulamanin ozel klasoru; baska uygulamalar erisemez. */
private fun settingsFilePath(context: Context): String =
    context.filesDir.resolve(SETTINGS_FILE_NAME).absolutePath
