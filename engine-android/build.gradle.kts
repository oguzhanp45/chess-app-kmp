plugins {
    alias(libs.plugins.androidLibrary)
}

// ============================================================
//  engine-android -- C++ motorun Android koprusu
// ============================================================
// Neden ayri bir modul: shared/ modulu AGP 9'un
// com.android.kotlin.multiplatform.library eklentisini kullaniyor ve
// bu eklenti externalNativeBuild / CMake / NDK / jniLibs desteklemiyor.
// Google'in onerdigi cozum, NDK isini klasik bir com.android.library
// modulunde tutup KMP modulunun androidMain'inden bagimlilik olarak
// kullanmak. Boylece JNI'in tamami tek modulde kapali kaliyor.

android {
    namespace = "com.oguzhanp.chess.engine"
    compileSdk = libs.versions.android.compileSdk.get().toInt()

    defaultConfig {
        minSdk = libs.versions.android.minSdk.get().toInt()

        // arm64-v8a  = Xiaomi Pad 7 (gercek test cihazi)
        // x86_64     = Pixel 7 emulatoru
        // Yayinda armeabi-v7a acilip x86_64 cikarilacak (Faz 8).
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                // engine/ kopyasinda tests/ ve uci/ yok. Motorun kendi
                // CMakeLists'i ANDROID tanimliyken bunlari zaten atliyor;
                // bu satir niyeti acik yaziyor.
                arguments += "-DCHESS_BUILD_TOOLS=OFF"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            // Android SDK ile gelen surum. Motor CMake 3.16+ istiyor.
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}
