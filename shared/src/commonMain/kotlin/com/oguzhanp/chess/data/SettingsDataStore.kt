package com.oguzhanp.chess.data

import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.PreferenceDataStoreFactory
import androidx.datastore.preferences.core.Preferences
import okio.Path.Companion.toPath


/**
 * Ayar dosyasinin adi. Degistirilirse kullanicinin kayitli ayarlari
 * kaybolur -- bir kez secilir ve dokunulmaz.
 *
 * `_pb` uzantisi gelenek: DataStore veriyi Protocol Buffers ile yaziyor.
 */
internal const val SETTINGS_FILE_NAME = "chess.preferences_pb"

/**
 * @param producePath dosyanin tam yolunu ureten fonksiyon; her platform
 *   kendi klasorunu biliyor ve burayi doldurmakla yukumlu.
 */
internal fun createSettingsDataStore(producePath: () -> String): DataStore<Preferences> =
    PreferenceDataStoreFactory.createWithPath(
        produceFile = { producePath().toPath() },
    )
