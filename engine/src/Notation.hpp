#pragma once
#include "Types.hpp"
#include "Board.hpp"
#include "MoveGen.hpp"
#include <string>
#include <vector>

// ============================================================
//  GOSTERIM KATMANI
// ============================================================
// moveToString ve parseMove daha once main.cpp icindeydi ve Tests.cpp
// bunlara "extern" ile ulasiyordu. Motoru bir kutuphane olarak
// derleyebilmek icin (CMake ve ileride mobil kopru) bu bagimliligin
// tersine cevrilmesi gerekiyordu: artik gosterim de kutuphanenin parcasi.
namespace Notation {

    // Uzun cebirsel gosterim (UCI): "e2e4", "e7e8q", bos hamle icin "0000"
    std::string toUci(const Move& m);

    // UCI metnini legal hamleye cevirir. Sinir kontrolu yapar ve hamlenin
    // gercekten legal oldugunu dogrular; gecersizse false doner ve tahtaya
    // dokunmaz.
    bool fromUci(const std::string& text, ChessBoard& board, MoveGenerator& moveGen, Move& out);

    // ============================================================
    //  STANDART CEBIRSEL GOSTERIM (SAN)
    // ============================================================
    // Insanlar "g1f3" degil "Nf3" okur. Mobil arayuzun hamle listesi,
    // PGN disa aktarimi ve analiz ekrani bunu kullanacak.
    //
    // Kapsam: tas harfi, belirsizlik giderme (Nbd2 / N1d2 / Nb1d2),
    // alim (x), piyon alimi (exd5), terfi (e8=Q), rok (O-O, O-O-O),
    // sah (+) ve mat (#).
    //
    // DIKKAT: hamle HENUZ OYNANMAMIS olmali; fonksiyon tahtayi gecici
    // olarak degistirip eski haline dondurur.
    std::string toSan(const Move& m, ChessBoard& board, MoveGenerator& moveGen);

    // Bir hamle dizisini SAN listesine cevirir (tahtayi eski haline birakir).
    std::vector<std::string> lineToSan(const std::vector<Move>& moves, ChessBoard& board, MoveGenerator& moveGen);
}