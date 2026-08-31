# ============================================================
#  chessjni -- motoru saran paylasilan kutuphane
# ============================================================
# Bu dosyanin yeri : <depo-kok>/engine-android/src/main/cpp/CMakeLists.txt
#
# Katmanlar:
#   engine/            C++ motor (kopya, dokunulmaz)  -> chessengine  [statik]
#   bridge/            extern "C" yuzeyi              -> chessbridge  [statik]
#   bu klasor          JNI shim'i                     -> chessjni     [.so]

cmake_minimum_required(VERSION 3.16)
project(chessjni LANGUAGES CXX)

# Motorun kendi CMakeLists'i C++17'yi kendi hedefi icin ayarliyor, ama
# o ayar alt dizin kapsaminda kaliyor. Bizim hedeflerimiz de
# EngineApi.hpp'yi dahil ettigi icin burada da acikca soylemek gerekiyor.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(CHESS_REPO_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Sira onemli: bridge, chessengine hedefini kullaniyor.
# Ikinci argumanlar zorunlu, cunku kaynak klasorler bu klasorun disinda.
add_subdirectory("${CHESS_REPO_ROOT}/engine" engine_build)
add_subdirectory("${CHESS_REPO_ROOT}/bridge" bridge_build)

add_library(chessjni SHARED
    chessjni.cpp
)

# chessbridge basliklarini PUBLIC verdigi icin ek ayar gerekmiyor.
target_link_libraries(chessjni PRIVATE chessbridge)
