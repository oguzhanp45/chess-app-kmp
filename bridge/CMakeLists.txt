# ============================================================
#  chessbridge -- motorun C yuzeyi
# ============================================================
# Statik kutuphane. Android tarafinda libchessjni.so'ya gomulur,
# iOS tarafinda cinterop'un baglanacagi kutuphane olur.
#
# chessengine hedefinin ONCE tanimlanmis olmasi gerekir; bu klasoru
# ekleyen CMakeLists once engine/ klasorunu ekler.

add_library(chessbridge STATIC
    chess_c_api.cpp
)

# PUBLIC: baglayan taraf ek ayar yapmadan #include "chess_c_api.h" yazabilir.
target_include_directories(chessbridge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# PUBLIC: chessengine'in basliklari da bagliyana gecer.
target_link_libraries(chessbridge PUBLIC chessengine)
