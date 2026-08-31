# ============================================================
#  chessjni -- motoru saran paylasilan kutuphane
# ============================================================
# Bu dosyanin yeri : <depo-kok>/engine-android/src/main/cpp/CMakeLists.txt
# Motor kaynaklari : <depo-kok>/engine/
#
# Uretilen: libchessjni.so  (icinde statik chessengine gomulu)

cmake_minimum_required(VERSION 3.16)
project(chessjni LANGUAGES CXX)

# Motorun kendi CMakeLists'i C++17'yi kendi hedefi icin ayarliyor, ama
# o ayar alt dizin kapsaminda kaliyor. Bizim hedefimiz de EngineApi.hpp'yi
# dahil ettigi icin burada da acikca soylemek zorundayiz.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(CHESS_REPO_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Motor deposunun CMakeLists'i birebir kopyalandi. ANDROID tanimli
# oldugu icin chessengine_tests ve UCI exe'si otomatik atlanir;
# yalnizca chessengine statik kutuphanesi derlenir.
# Ikinci arguman zorunlu: kaynak klasor bu klasorun disinda.
add_subdirectory("${CHESS_REPO_ROOT}/engine" engine_build)

add_library(chessjni SHARED
    chessjni.cpp
)

# chessengine baslik klasorunu PUBLIC veriyor, bu yuzden
# target_include_directories yazmamiza gerek yok.
target_link_libraries(chessjni PRIVATE chessengine)
