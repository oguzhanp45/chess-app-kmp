# ============================================================
#  chess-app-kmp -- masaustu arac derlemesi
# ============================================================
# Bu dosya UYGULAMAYI derlemez. Uygulama Gradle ile derlenir.
# Buradaki tek amac, C yuzeyini (chess_c_api) cihaza gitmeden
# masaustunde derleyip test edebilmek: capitest.
#
# Kullanim (Git Bash, depo kokunde):
#   cmake -B build-desktop -DCHESS_BUILD_TOOLS=OFF
#   cmake --build build-desktop --config Release
#   ./build-desktop/Release/capitest.exe
#
# -DCHESS_BUILD_TOOLS=OFF zorunlu: engine/ klasoru motor deposunun
# yalnizca src/ kismini iceriyor, tests/ ve uci/ kopyalanmadi.
#
# Android derlemesi buraya HIC BAKMAZ; Gradle dogrudan
# engine-android/src/main/cpp/CMakeLists.txt'i cagirir.

cmake_minimum_required(VERSION 3.16)
project(chess-app-kmp-desktop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_subdirectory(engine engine_build)
add_subdirectory(bridge bridge_build)
