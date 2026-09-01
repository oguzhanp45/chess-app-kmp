package com.oguzhanp.chess.data

import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.emptyPreferences
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.map

// ============================================================
//  SettingsRepository -- kullanici ayarlari
// ============================================================
// Mimari kural 9: ekranlar veri kaynagina DOGRUDAN konusmaz. Ekran
// DataStore diye bir sey bilmiyor, bu sinifi kullaniyor.
//
// Simdilik somut bir sinif, arayuz degil. Arayuzun faydasi ikinci bir
// uygulama oldugunda ortaya cikar (Faz B'de bulut senkronu). O gun
// gelince bu sinifin uyelerini bir arayuze cikarmak kisa bir is.
//
// Faz 7 notu: kalicilik katmani orada genisleyecek (Room, PGN, profil).
// Bu sinif o katmanin ilk parcasi -- Faz 7'de yeniden yazilmayacak,
// uzerine eklenecek.

class SettingsRepository(
    private val dataStore: DataStore<Preferences>,
) {

    /**
     * Kullanici tanitim ekranlarini gordu mu.
     *
     * Flow: deger diskten asenkron geliyor ve sonradan degisebiliyor.
     * Ilk deger anlik gelmez -- cagiran taraf "henuz bilmiyorum"
     * durumunu ele almak zorunda (App.kt'deki null hali).
     */
    val isOnboarded: Flow<Boolean> = dataStore.data
        // Dosya bozuk ya da okunamaz ise uygulama cokmesin; varsayilana
        // dus. Ayar dosyasi kritik veri degil, kaybi kabul edilebilir.
        .catch { emit(emptyPreferences()) }
        .map { preferences -> preferences[KEY_ONBOARDED] ?: false }

    suspend fun setOnboarded(value: Boolean) {
        dataStore.edit { preferences -> preferences[KEY_ONBOARDED] = value }
    }

    private companion object {
        // Anahtar dizesi de dosya adi gibi kalicidir: degistirirsen
        // eski kayit okunamaz hale gelir.
        val KEY_ONBOARDED = booleanPreferencesKey("is_onboarded")
    }
}
