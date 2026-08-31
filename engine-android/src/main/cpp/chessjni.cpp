// ============================================================
//  JNI shim -- Kotlin ile chess_c_api arasindaki ince katman
// ============================================================
// Bu dosyada satranc mantigi YOKTUR ve olmayacaktir. Tek isi
// Kotlin tiplerini C tiplerine cevirmek.
//
// JNI fonksiyon adlari Kotlin tarafindaki paket + sinif adindan
// mekanik olarak turer:
//
//   com.oguzhanp.chess.engine.NativeBridge.makeMove()
//     -> Java_com_oguzhanp_chess_engine_NativeBridge_makeMove
//
// Kotlin tarafinda paket ya da sinif adi degisirse buradaki adlar da
// degismeli; yoksa calisma aninda UnsatisfiedLinkError alinir.
// (Derleme sirasinda fark edilmez -- bag calisma aninda kuruluyor.)
//
// TUTAMAC: Kotlin tarafinda Long olarak duran deger, aslinda
// chess_create()'in dondurdugu isaretcinin kendisi. Kotlin icine hic
// bakmaz, yalnizca geri verir. 64 bitlik jlong hem arm64 hem x86_64
// isaretcisini tasir.

#include <jni.h>

#include <string>

#include "chess_c_api.h"

namespace {

// jstring'i C dizesine cevirir ve yikimda serbest birakir.
// GetStringUTFChars kopya yapabilir; Release cagrilmazsa sizinti olur.
class JniString {
public:
    JniString(JNIEnv* env, jstring value)
        : env_(env), value_(value), chars_(nullptr) {
        if (value_ != nullptr) {
            chars_ = env_->GetStringUTFChars(value_, nullptr);
        }
    }

    ~JniString() {
        if (chars_ != nullptr) {
            env_->ReleaseStringUTFChars(value_, chars_);
        }
    }

    JniString(const JniString&) = delete;
    JniString& operator=(const JniString&) = delete;

    const char* c_str() const { return chars_ != nullptr ? chars_ : ""; }

private:
    JNIEnv* env_;
    jstring value_;
    const char* chars_;
};

inline chess_engine* toEngine(jlong handle) {
    return reinterpret_cast<chess_engine*>(handle);
}

} // namespace

// ------------------------------------------------------------
//  Surum ve oz test (tutamac gerektirmez)
// ------------------------------------------------------------

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

// ------------------------------------------------------------
//  Tutamac yonetimi
// ------------------------------------------------------------

extern "C" JNIEXPORT jlong JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_create(JNIEnv* /*env*/, jobject /*thiz*/) {
    return reinterpret_cast<jlong>(chess_create());
}

extern "C" JNIEXPORT void JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_destroy(JNIEnv* /*env*/, jobject /*thiz*/,
                                                    jlong handle) {
    chess_destroy(toEngine(handle));
}

// ------------------------------------------------------------
//  Oyun kurulumu ve durum
// ------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_newGame(JNIEnv* env, jobject /*thiz*/,
                                                    jlong handle, jstring fen) {
    JniString value(env, fen);
    return chess_new_game(toEngine(handle), value.c_str()) != 0 ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_snapshotJson(JNIEnv* env, jobject /*thiz*/,
                                                         jlong handle) {
    return env->NewStringUTF(chess_snapshot_json(toEngine(handle)));
}

// ------------------------------------------------------------
//  Hamle
// ------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_makeMove(JNIEnv* env, jobject /*thiz*/,
                                                     jlong handle, jstring uci) {
    JniString value(env, uci);
    return chess_make_move(toEngine(handle), value.c_str()) != 0 ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_undo(JNIEnv* /*env*/, jobject /*thiz*/,
                                                 jlong handle) {
    return chess_undo(toEngine(handle)) != 0 ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_sanFor(JNIEnv* env, jobject /*thiz*/,
                                                   jlong handle, jstring uci) {
    JniString value(env, uci);
    return env->NewStringUTF(chess_san_for(toEngine(handle), value.c_str()));
}
