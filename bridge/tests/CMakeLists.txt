# capitest -- yalnizca masaustu. Mobil hedeflerde bu klasor eklenmez.
add_executable(capitest capitest.cpp)
target_link_libraries(capitest PRIVATE chessbridge)

# ctest'e bagliyoruz: uc isletim sisteminde de tek komutla kosuyor
# (Windows'ta cikti Release/ alt klasorune girdigi icin yol degisiyor,
# ctest bunu kendi cozuyor).
add_test(NAME capitest COMMAND capitest)
