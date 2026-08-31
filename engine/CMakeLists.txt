# ============================================================
#  cpp-chess-engine - CMake yapilandirmasi
# ============================================================
# Uc hedef uretir:
#
#   chessengine        statik kutuphane, src/*        -> masaustu VE mobil
#   chessengine_tests  statik kutuphane, tests/*      -> yalnizca masaustu
#   cpp-chess-engine   calistirilabilir, uci/main.cpp -> yalnizca masaustu
#
# Testler ve UCI katmani ayri hedeflerde: mobil uygulama yalnizca
# chessengine'i baglar, 33 KB'lik test kodu APK'ya girmez.
#
# Masaustu kullanim (Git Bash, depo kokunde):
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --config Release
#
# Mobil kullanim (Android NDK / iOS): ANDROID veya IOS tanimliyken
# son iki hedef otomatik atlanir. Elle kapatmak icin:
#   -DCHESS_BUILD_TOOLS=OFF
cmake_minimum_required(VERSION 3.16)
project(cpp-chess-engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tek yapilandirmali ureticilerde (Ninja, Makefiles) varsayilan Release.
# Visual Studio cok yapilandirmalidir, orada bu satir devre disi kalir.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

option(CHESS_BUILD_TOOLS "Testleri ve UCI uygulamasini derle" ON)
if(ANDROID OR IOS)
    set(CHESS_BUILD_TOOLS OFF)
endif()

# ------------------------------------------------------------
#  1) chessengine - motor cekirdegi (mobil bunu baglar)
# ------------------------------------------------------------
add_library(chessengine STATIC
    src/Board.cpp
    src/MoveGen.cpp
    src/AI.cpp
    src/Polyglot.cpp
    src/Notation.cpp
    src/EngineApi.cpp
)

# PUBLIC: bu hedefi baglayan herkes src/ klasorunu de gorur.
# Boylece uci/ ve tests/ icinden #include "EngineApi.hpp" calisir.
target_include_directories(chessengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Threads REQUIRED)
target_link_libraries(chessengine PUBLIC Threads::Threads)

# Ortak derleyici bayraklari.
# /Oi  = intrinsic fonksiyonlar (eski .vcxproj'de IntrinsicFunctions=true)
# /Gy  = fonksiyon duzeyinde baglama (eski .vcxproj'de FunctionLevelLinking=true)
# Bu ikisi eski Visual Studio projesinde acikti ama CMake varsayilaninda yok.
if(MSVC)
    set(CHESS_FLAGS /O2 /Oi /Gy /W3 /utf-8)
else()
    set(CHESS_FLAGS -O3 -Wall -Wextra)
endif()

target_compile_options(chessengine PRIVATE ${CHESS_FLAGS})

# ------------------------------------------------------------
#  2) chessengine_tests + 3) cpp-chess-engine - masaustu
# ------------------------------------------------------------
if(CHESS_BUILD_TOOLS)

    add_library(chessengine_tests STATIC
        tests/Tests.cpp
    )
    target_include_directories(chessengine_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(chessengine_tests PUBLIC chessengine)

    add_executable(cpp-chess-engine
        uci/main.cpp
    )
    target_link_libraries(cpp-chess-engine PRIVATE chessengine_tests)

    target_compile_options(chessengine_tests PRIVATE ${CHESS_FLAGS})
    target_compile_options(cpp-chess-engine  PRIVATE ${CHESS_FLAGS})

    # --- Tum program optimizasyonu (MSVC /GL + /LTCG, GCC/Clang -flto) ---
    # Eski .vcxproj'de WholeProgramOptimization=true idi; CMake bunu
    # varsayilan olarak ACMIYOR. Satranc motorunda search() baska bir
    # ceviri biriminde duran evaluateBoard() ve orderMoves()'u milyonlarca
    # kez cagirdigi icin ceviri birimleri arasi inline etme cok is goruyor.
    #
    # Yalnizca masaustunde aciyoruz: mobilde LTO baglama suresini uzatiyor
    # ve NDK/Xcode tarafinda ayrica dogrulanmasi gerekir. Faz 1'de bakariz.
    include(CheckIPOSupported)
    check_ipo_supported(RESULT CHESS_IPO_OK OUTPUT CHESS_IPO_MSG)
    if(CHESS_IPO_OK)
        set_property(TARGET chessengine       PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET chessengine_tests PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET cpp-chess-engine  PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        message(STATUS "Tum program optimizasyonu (IPO/LTO): ACIK")
    else()
        message(STATUS "Tum program optimizasyonu (IPO/LTO): KAPALI - ${CHESS_IPO_MSG}")
    endif()

endif()
