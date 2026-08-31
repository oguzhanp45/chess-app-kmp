#!/usr/bin/env bash
# ============================================================
#  check-jni-symbols.sh
# ============================================================
# NativeBridge.kt'deki her `external fun` icin libchessjni.so'da
# karsilik gelen bir sembol var mi diye bakar.
#
# NEDEN: JNI bagi CALISMA ANINDA kuruluyor. Kotlin tarafinda bir ad
# degistirip C++ tarafini unutursan uygulama sorunsuz derlenir, sonra
# o fonksiyon ilk cagrildiginda UnsatisfiedLinkError ile coker --
# belki de kullanicinin elinde. Bu betik o hatayi derleme zamanina ceker.
#
# Kullanim (depo kokunde):
#   ./gradlew :engine-android:assembleDebug
#   ./tools/check-jni-symbols.sh
#
# Elle yol vermek icin:
#   ./tools/check-jni-symbols.sh <NativeBridge.kt> <libchessjni.so>
#
# Sembol okuyucu: Linux/macOS'ta readelf ya da nm hazir gelir. Windows'ta
# Git Bash'te ikisi de yoktur; betik NDK'nin icindeki llvm-readelf'i
# ANDROID_HOME uzerinden bulur.

set -euo pipefail

KT="${1:-shared/src/androidMain/kotlin/com/oguzhanp/chess/engine/NativeBridge.kt}"
SO="${2:-}"
PREFIX="Java_com_oguzhanp_chess_engine_NativeBridge_"

if [ ! -f "$KT" ]; then
    echo "HATA: $KT bulunamadi" >&2
    exit 1
fi

if [ -z "$SO" ]; then
    SO=$(find engine-android/build -name 'libchessjni.so' -path '*arm64-v8a*' 2>/dev/null | head -1)
fi

if [ -z "$SO" ] || [ ! -f "$SO" ]; then
    echo "HATA: libchessjni.so bulunamadi. Once derle:" >&2
    echo "      ./gradlew :engine-android:assembleDebug" >&2
    exit 1
fi

# ------------------------------------------------------------
#  Sembol okuyucuyu bul
# ------------------------------------------------------------
find_symbol_tool() {
    local candidate
    for candidate in llvm-readelf readelf llvm-nm nm; do
        if command -v "$candidate" > /dev/null 2>&1; then
            echo "$candidate"
            return 0
        fi
    done

    # Hicbiri yoksa NDK'nin icine bak (Windows'ta olagan durum).
    local sdk="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-}}"
    [ -z "$sdk" ] && return 1

    # Windows'ta ANDROID_HOME "C:\..." bicimindedir; find POSIX yol ister.
    if command -v cygpath > /dev/null 2>&1; then
        sdk=$(cygpath -u "$sdk")
    fi
    [ -d "$sdk/ndk" ] || return 1

    find "$sdk/ndk" -type f \( -name 'llvm-readelf' -o -name 'llvm-readelf.exe' \) \
        2>/dev/null | head -1
}

TOOL=$(find_symbol_tool || true)

if [ -z "$TOOL" ]; then
    echo "HATA: sembol okuyucu bulunamadi (readelf / nm / llvm-readelf)." >&2
    echo "      ANDROID_HOME ayarli mi ve NDK kurulu mu diye bak." >&2
    exit 1
fi

case "$(basename "$TOOL")" in
    *nm*) TOOL_ARGS="-D" ;;
    *)    TOOL_ARGS="--dyn-syms -W" ;;
esac

echo "Kotlin : $KT"
echo "Sembol : $SO"
echo "Arac   : $TOOL"
echo

# Aracin cikti bicimine bagimli kalmamak icin dogrudan Java_ ile baslayan
# adlari topluyoruz -- readelf ve nm ciktilarinin ikisinde de calisir.
SYMBOLS=$("$TOOL" $TOOL_ARGS "$SO" 2>/dev/null | grep -oE 'Java_[A-Za-z0-9_]+' | sort -u)

if [ -z "$SYMBOLS" ]; then
    echo "HATA: .so icinde hic Java_ sembolu bulunamadi." >&2
    echo "      Arac ($TOOL) bu dosyayi okuyamamis olabilir." >&2
    exit 1
fi

missing=0
total=0

while read -r name; do
    [ -z "$name" ] && continue
    total=$((total + 1))
    if grep -qx "${PREFIX}${name}" <<< "$SYMBOLS"; then
        printf '  %-24s tamam\n' "$name"
    else
        printf '  %-24s EKSIK -> %s%s\n' "$name" "$PREFIX" "$name"
        missing=$((missing + 1))
    fi
done < <(grep -oE 'external fun [A-Za-z0-9_]+' "$KT" | awk '{ print $3 }')

echo
if [ "$missing" -eq 0 ]; then
    echo "JNI sembolleri: $total/$total eslesti"
    exit 0
fi

echo "JNI sembolleri: $((total - missing))/$total eslesti -- $missing EKSIK" >&2
echo "Kotlin tarafindaki ad ile chessjni.cpp'deki ad uyusmuyor." >&2
exit 1
