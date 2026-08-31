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

// ------------------------------------------------------------
//  Arama -- BLOKE EDER, Kotlin tarafinda arka planda cagrilmali
// ------------------------------------------------------------

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_bestMove(JNIEnv* env, jobject /*thiz*/,
                                                     jlong handle, jint timeMs,
                                                     jint maxDepth) {
    return env->NewStringUTF(chess_best_move(toEngine(handle), timeMs, maxDepth));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_bestMovesJson(JNIEnv* env, jobject /*thiz*/,
                                                          jlong handle, jint n,
                                                          jint timeMs, jint maxDepth) {
    return env->NewStringUTF(chess_best_moves_json(toEngine(handle), n, timeMs, maxDepth));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_evaluateJson(JNIEnv* env, jobject /*thiz*/,
                                                         jlong handle) {
    return env->NewStringUTF(chess_evaluate_json(toEngine(handle)));
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_bookMovesJson(JNIEnv* env, jobject /*thiz*/,
                                                          jlong handle) {
    return env->NewStringUTF(chess_book_moves_json(toEngine(handle)));
}

// Arama surerken BASKA BIR IS PARCACIGINDAN cagrilir. Motor tarafinda
// yalnizca atomik bir bayrak set ediyor, o yuzden guvenli.
extern "C" JNIEXPORT void JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_stop(JNIEnv* /*env*/, jobject /*thiz*/,
                                                 jlong handle) {
    chess_stop(toEngine(handle));
}

// ------------------------------------------------------------
//  Canli arama bilgisi -- arama surerken cagrilabilir
// ------------------------------------------------------------
// Bunlar motora dokunmuyor; C katmanindaki atomik alanlari okuyorlar.

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoDepth(JNIEnv*, jobject, jlong handle) {
    return chess_info_depth(toEngine(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoSelDepth(JNIEnv*, jobject, jlong handle) {
    return chess_info_sel_depth(toEngine(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoScoreCp(JNIEnv*, jobject, jlong handle) {
    return chess_info_score_cp(toEngine(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoMateIn(JNIEnv*, jobject, jlong handle) {
    return chess_info_mate_in(toEngine(handle));
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoNodes(JNIEnv*, jobject, jlong handle) {
    return static_cast<jlong>(chess_info_nodes(toEngine(handle)));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_infoTimeMs(JNIEnv*, jobject, jlong handle) {
    return chess_info_time_ms(toEngine(handle));
}

// ------------------------------------------------------------
//  Son aramanin sonucu
// ------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_lastScore(JNIEnv*, jobject, jlong handle) {
    return chess_last_score(toEngine(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_lastDepth(JNIEnv*, jobject, jlong handle) {
    return chess_last_depth(toEngine(handle));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_lastSkillLoss(JNIEnv*, jobject, jlong handle) {
    return chess_last_skill_loss(toEngine(handle));
}

// ------------------------------------------------------------
//  Ayarlar
// ------------------------------------------------------------

extern "C" JNIEXPORT void JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_setSkillLevel(JNIEnv*, jobject,
                                                          jlong handle, jint level) {
    chess_set_skill_level(toEngine(handle), level);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_getSkillLevel(JNIEnv*, jobject, jlong handle) {
    return chess_get_skill_level(toEngine(handle));
}

extern "C" JNIEXPORT void JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_setHashMb(JNIEnv*, jobject,
                                                      jlong handle, jint mb) {
    chess_set_hash_mb(toEngine(handle), mb);
}

extern "C" JNIEXPORT void JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_setUseBook(JNIEnv*, jobject,
                                                       jlong handle, jboolean on) {
    chess_set_use_book(toEngine(handle), on == JNI_TRUE ? 1 : 0);
}

// Kitap APK varligi oldugu icin Kotlin tarafinda ByteArray olarak okunur.
extern "C" JNIEXPORT jboolean JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_loadBookFromMemory(JNIEnv* env, jobject /*thiz*/,
                                                               jlong handle,
                                                               jbyteArray bytes) {
    if (bytes == nullptr) return JNI_FALSE;

    const jsize size = env->GetArrayLength(bytes);
    jbyte* data = env->GetByteArrayElements(bytes, nullptr);
    if (data == nullptr) return JNI_FALSE;

    const int ok = chess_load_book_from_memory(
        toEngine(handle),
        reinterpret_cast<const unsigned char*>(data),
        static_cast<size_t>(size));

    // JNI_ABORT: diziyi degistirmedik, JVM'e geri yazmasina gerek yok.
    env->ReleaseByteArrayElements(bytes, data, JNI_ABORT);

    return ok != 0 ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_oguzhanp_chess_engine_NativeBridge_isBookLoaded(JNIEnv*, jobject, jlong handle) {
    return chess_is_book_loaded(toEngine(handle)) != 0 ? JNI_TRUE : JNI_FALSE;
}
