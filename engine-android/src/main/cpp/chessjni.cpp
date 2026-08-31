// ============================================================
//  JNI shim -- Kotlin ile chess_c_api arasindaki ince katman
// ============================================================
// Bu dosyada satranc mantigi YOKTUR ve olmayacaktir. Tek isi
// Kotlin tiplerini C tiplerine cevirmek.
//
// JNI fonksiyon adlari Kotlin tarafindaki paket + sinif adindan
// mekanik olarak turer:
//
//   com.oguzhanp.chess.engine.NativeBridge.version()
//     -> Java_com_oguzhanp_chess_engine_NativeBridge_version
//
// Kotlin tarafinda paket ya da sinif adi degisirse buradaki adlar da
// degismeli; yoksa calisma aninda UnsatisfiedLinkError alinir.
// (Derleme sirasinda fark edilmez -- bag calisma aninda kuruluyor.)

#include <jni.h>

#include "chess_c_api.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_version(JNIEnv* env, jobject /*thiz*/) {
    // NewStringUTF C dizesini JVM dizesine KOPYALAR. chess_c_api'nin
    // bellek sahipligi kurali tam da bu yuzden yeterli: isaretcinin
    // bir sonraki cagriya kadar yasamasi kafi.
    return env->NewStringUTF(chess_version());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_selftest(JNIEnv* /*env*/, jobject /*thiz*/) {
    return static_cast<jint>(chess_selftest());
}
