package com.oguzhanp.chess.data

import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.cinterop.ExperimentalForeignApi
import platform.Foundation.NSDocumentDirectory
import platform.Foundation.NSFileManager
import platform.Foundation.NSURL
import platform.Foundation.NSUserDomainMask

// Android tarafiyla ayni gerekce: surec basina tek ornek.
private var instance: SettingsRepository? = null

@Composable
internal actual fun rememberSettingsRepository(): SettingsRepository =
    remember {
        instance ?: SettingsRepository(
            createSettingsDataStore { settingsFilePath() }
        ).also { instance = it }
    }

/**
 * iOS'ta uygulamanin Documents klasoru.
 *
 * platform.Foundation.* siniflarini biz yazmadik: Kotlin/Native, iOS
 * SDK basliklarini okuyup Kotlin karsiliklarini kendisi uretiyor.
 *
 * @OptIn(ExperimentalForeignApi): URLForDirectory'nin error parametresi
 * C tarafinda bir isaretci. Kotlin/Native'in isaretci API'si hala
 * deneysel isaretli oldugu icin acik izin istiyor.
 */
@OptIn(ExperimentalForeignApi::class)
private fun settingsFilePath(): String {
    val documentDirectory: NSURL? = NSFileManager.defaultManager.URLForDirectory(
        directory = NSDocumentDirectory,
        inDomain = NSUserDomainMask,
        appropriateForURL = null,
        create = false,
        error = null,
    )
    val path = requireNotNull(documentDirectory?.path) {
        "iOS: Documents klasoru bulunamadi"
    }
    return "$path/$SETTINGS_FILE_NAME"
}
