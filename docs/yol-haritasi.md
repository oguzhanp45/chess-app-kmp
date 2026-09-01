# Yol haritası — chess-app-kmp

Bu dosya planın tek kaydı. Sohbette konuşulan her karar buraya düşer;
"bunu ne zaman konuşmuştuk" sorusunun cevabı burasıdır.

Kural: **bir iş bitince satırı işaretle, yeni bir iş çıkınca listeye yaz.**
Sonradan eklenen işler en alttaki bölümde toplanır ve oradan uygun faza
taşınır.

---

## Durum özeti

| Faz | İş | Durum |
|---|---|---|
| 1 | KMP iskeleti + C++ köprüsü | ✅ bitti |
| 2 | Motor sarmalayıcısı (coroutine, StateFlow, veri sınıfları) | ✅ bitti |
| — | Onboarding + tema + ayar kalıcılığı | ✅ bitti *(plan dışı, aşağıda)* |
| 3 | Tahta (Compose) | ⬜ sırada |
| 4 | Oyun akışı | ⬜ |
| 5 | Navigasyon ve 5 sekme | ⬜ |
| 6 | Dil + açılış kitabı + ses | ⬜ |
| 7 | Kalıcılık (Room, PGN, profil) | 🟡 bir dilimi yapıldı |
| 8 | Cila + release APK → **Faz A kesim noktası** | ⬜ |

---

## Faz 1 — KMP iskeleti ve köprü ✅

| | | |
|---|---|---|
| 1.1 | GitHub deposu + `git init` | ✅ |
| 1.2 | Android Studio KMP sihirbazı | ✅ |
| 1.3 | Motor kaynaklarını `engine/`'e kopyala | ✅ |
| 1.4 | NDK/CMake — `engine-android` modülü | ✅ |
| 1.5 | `chess_c_api` + JNI shim, ekranda sürüm | ✅ |
| 1.6 | Tutamaç tabanlı yüzey + `ChessEngine` | ✅ |
| 1.7 | GitHub Actions (capitest ×3 OS, Android, iOS) | ✅ |

## Faz 2 — Motor sarmalayıcısı ✅

| | | |
|---|---|---|
| 2.1 | Arama, canlı bilgi, ayarlar → capitest 55 kontrol | ✅ |
| 2.2 | JNI genişletme + sembol denetleyicisi (29/29) | ✅ |
| 2.3 | kotlinx.serialization + veri sınıfları + 11 test | ✅ |
| 2.4 | Coroutine `ChessEngine` + `ViewModel` + doğrulama ekranı | ✅ |

## Faz 3 — Tahta ⬜

Kararlar alındı (hamle etkileşimi, taş çizimi, vurgular, koordinatlar).

| | | |
|---|---|---|
| 3.0 | **Tasarım sistemi** — tipografi, boşluk ölçeği, köşe yarıçapları | ⬜ *önerildi* |
| 3.1 | Ölçü/renk sabitleri, kareler, koordinatlar, FEN→tahta, `PieceSet` + Unicode | ⬜ |
| 3.2 | Vektör taş seti | ⬜ |
| 3.3 | Seçim, legal hamle noktaları, tıkla-oyna | ⬜ |
| 3.4 | Sürükle-bırak + kayma animasyonu | ⬜ |

**Alınmış kararlar:**
- Hamle: hem tıkla-seç/tıkla-oyna hem sürükle-bırak
- Taşlar: `PieceSet` arayüzü arkasında; önce Unicode, sonra vektör
- Legal hamle: boş karede nokta, alınabilir taşta halka
- Koordinatlar: karenin köşesinde, küçük ve soluk
- Tahta üç katman: kareler / vurgular / taşlar

## Faz 4 — Oyun akışı ⬜

Kurulum ekranı, saat, terfi penceresi, oyun sonu, yerel 1v1.

- Saat: siyah üstte, beyaz altta (chess.com tarzı)
- Motorun düşünme süresi kalan süreden hesaplanacak
  *(şu an `ENGINE_THINK_MS = 4000` sabit)*

## Faz 5 — Navigasyon ve 5 sekme ⬜

Ana Sayfa · Oyna · Bulmacalar · Öğren · Daha Fazla

- `navigation-compose` (KMP) burada eklenecek — onboarding için bilerek ertelendi
- Dar ekranda alt sekme çubuğu, geniş ekranda sol kenar çubuğu
- RN'deki iki kusur tekrarlanmayacak: yan çubuk fazla geniş, aktif sekme
  rengi hangi sekmede olduğunu gizliyor
- `debug/EngineTestScreen.kt` burada **silinecek**

## Faz 6 — Dil, kitap, ses ⬜

- Dil (tr/en): ekrandaki her metin sözlükten gelecek (mimari kural 7).
  Şu an sabit metinler: `OnboardingScreen`, `EngineTestScreen`,
  `EngineViewModel.levelName`
- Açılış kitabı: `book.bin` commit'lenecek, Android'de `assets/` altına;
  `chess_load_book_from_memory` köprüde hazır
- Ses: 14 ses kimliği (RN sürümünden)

## Faz 7 — Kalıcılık 🟡

| | | |
|---|---|---|
| 7.x | `SettingsRepository` + DataStore | ✅ **öne alındı** |
| | Room, oyun geçmişi, PGN, yerel profil | ⬜ |
| | Depo arayüzleri (`GameRepository`, `ProfileRepository`, `PuzzleRepository`) | ⬜ |

`SettingsRepository` yeniden yazılmayacak, üzerine eklenecek. Bugün somut
bir sınıf; ikinci bir uygulama (bulut) gerektiğinde arayüze çıkarılacak.

Senkron alanları: `uuid`, `updatedAt`, `deleted`, `syncedAt`.

## Faz 8 — Cila ⬜

- Uygulama adı belirlenecek
- `abiFilters` `androidApp`'e de eklenecek *(şu an sadece `engine-android`'de;
  release APK'da armeabi-v7a/x86 dilimlerinde `libchessjni.so` yok)*
- Kod küçültme açılırsa `NativeBridge` için keep kuralı gerekecek
- Giriş ekranı **sadece kabuk** — sahte giriş yok
- **Sistem splash'ı ile bizim splash'ımızı hizala.** Android 12'den beri
  sistem uygulama simgesini Compose başlamadan gösteriyor; şu an iki
  açılış ekranı üst üste biniyor
- **Uygulama içi "Açık kaynak lisansları" ekranı** (Daha Fazla → Destek).
  İçerik `docs/lisanslar.md`'de hazır
- Release APK + tablette gerçek kullanım testi

---

## Plan dışı eklenenler

Sonradan çıkan, özgün faz listesinde olmayan işler.

### ✅ Onboarding (2026-09)

3 ekran, swipe, "Geç" tuşu, `isOnboarded` ile bir kez gösterme.
Dosyalar: `onboarding/OnboardingScreen.kt`, `App.kt`, `data/Settings*`.

Navigation kullanılmadı — iki hedef için gereksiz, Faz 5'e bırakıldı.

### ✅ Açılış animasyonu (2026-09)

Logo 500 ms'de belirip büyüyor, 200 ms duruyor. Yapay gecikme yok:
veri geldiğinde **ve** animasyon bittiğinde geçiliyor.

### ✅ Onboarding görselleri (2026-09)

Sayfa başına ayrı animasyon, hepsi Compose ilkelleriyle çizildi
(dış görsel dosyası yok): motor seviyeleri · puan artışı · çevrimdışı.

**Uygulama içeriği tamamlanınca bu görseller ve metinler yeniden
gözden geçirilecek** — o zaman elde gerçek özellikler olacak.

### ✅ Tema altyapısı (2026-09)

`theme/ChessColors.kt` + `theme/ChessTheme.kt`. Cihazın açık/koyu moduna
uyuyor. Yön: klasik ve lüks — sıcak fildişi zemin, bronz/altın vurgu.

Uygulama içi tema seçimi Faz 6/8'e bırakıldı; `ChessTheme`'in `darkTheme`
parametresi şimdiden hazır.

### ⬜ Tasarım sistemi *(konuşulacak)*

Renkler var ama tipografi, boşluk ölçeği ve köşe yarıçapları yok. Tahtayı
yapmadan önce oturtulursa Faz 3-5 boyunca dağılmaz. Faz 3.0 olarak
işaretlendi.

---

## Faz A sonrası (yönü belli, sırası değil)

Hesap ve senkron (Firebase Auth düşünülüyor) · bulmacalar ve Lichess API ·
analiz (eval bar, varyantlar, hamle kalitesi) · eğitmen (C++ etiketleri +
seviyeye göre şablon cümle + TTS, LLM yok) · öğren (Satranç Yolculuğu,
Ustalardan Dersler) · profil ve istatistik · ekstra modlar.

---

## Süreklilik kuralları

Her adımda uygulanacak, unutulmaması gereken işler.

- **Yeni bir görsel, ikon veya font eklendiğinde** `docs/lisanslar.md`
  güncellenir. Tek satırlık iş; unutulursa toplaması zor
- **"Bunu sonra yaparız" denen her şey** bu dosyaya yazılır — konuşmada
  kalmaz
- **Yeni sabit metin yazıldığında** Faz 6 bölümüne hangi dosyada olduğu
  not edilir (sözlüğe taşınacak)
- **Her büyük adım sonunda** commit + push; CI yeşil olmadan sonraki adıma
  geçilmez
- **Motor tarafında bir değişiklik olursa** `engine/VERSION.md` güncellenir
- Claude bir dosya yazdığında **boyutunu söyler**; dosya Android Studio'da
  açıksa editör üzerine yazabiliyor, `File → Reload from Disk` gerekiyor

---

## Değişmez kurallar

1. C++ motoru tek doğru kaynaktır
2. Satranç kuralı Kotlin'de yok
3. Arama sürerken motora dokunulmaz *(artık yapısal: tek slotlu dağıtıcı)*
4. Köprüden geçen her şey basit tip; hamle başına bir geçiş
5. Her katman yalnızca bir altındakini tanır
6. Renk ve ölçü sabitleri tek dosyada
7. Ekranda görünen her metin sözlükten gelir (tr/en)
8. Çevrimdışı öncelikli
9. Ekranlar veritabanına doğrudan konuşmaz
