/*! \file
    \brief Битовые утилиты для marty::mem
 */

#pragma once

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#include "assert.h"
#include "fixed_size_types.h"

//------------------------------
#include <algorithm>
#include <string>
#include <unordered_map>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/bits.h"
// marty::mem::bits::
namespace marty{
namespace mem{
namespace bits{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// How do I convert between big-endian and little-endian values in C++? - https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c/105371#105371
// GCC intrinsics - https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
// TODO: use intrinsics if available

inline uint8_t  swapBytes(uint8_t u)   { return u; }
inline uint16_t swapBytes(uint16_t u)  { return uint16_t((u>>8) | (u<<8)); }
inline uint32_t swapBytes(uint32_t u)  { return uint32_t(swapBytes(uint16_t(u>>16))) | uint32_t(uint32_t(swapBytes(uint16_t(u))) << 16); }
inline uint64_t swapBytes(uint64_t u)  { return uint64_t(swapBytes(uint32_t(u>>32))) | uint64_t(uint64_t(swapBytes(uint32_t(u))) << 32); }

inline int8_t   swapBytes(int8_t u)    { return int8_t (swapBytes(uint8_t(u))) ; }
inline int16_t  swapBytes(int16_t u)   { return int16_t(swapBytes(uint16_t(u))); }
inline int32_t  swapBytes(int32_t u)   { return int32_t(swapBytes(uint32_t(u))); }
inline int64_t  swapBytes(int64_t u)   { return int64_t(swapBytes(uint64_t(u))); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline unsigned countOnes(uint8_t v)
{
  constexpr static const unsigned table[16] =
  {
    0, 1, 1, 2, 1, 2, 2, 3, 
    1, 2, 2, 3, 2, 3, 3, 4
  };

  return table[v&0x0F] + table[v>>4];
}

inline unsigned countOnes(uint16_t v)  { return countOnes(uint8_t (v>> 8)) + countOnes(uint8_t (v)); }
inline unsigned countOnes(uint32_t v)  { return countOnes(uint16_t(v>>16)) + countOnes(uint16_t(v)); }
inline unsigned countOnes(uint64_t v)  { return countOnes(uint32_t(v>>32)) + countOnes(uint32_t(v)); }

inline unsigned countOnes(int8_t   v)  { return countOnes(uint8_t (v)); }
inline unsigned countOnes(int16_t  v)  { return countOnes(uint16_t(v)); }
inline unsigned countOnes(int32_t  v)  { return countOnes(uint32_t(v)); }
inline unsigned countOnes(int64_t  v)  { return countOnes(uint64_t(v)); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//! returns most significant bit power
constexpr inline int getMsbPower(uint8_t v)
{
    return (v>=0x80u)
           ? 7
           : (v>=0x40u)
             ? 6
             : (v>=0x20u)
               ? 5
               : (v>=0x10u)
                 ? 4
                 : (v>=0x08u)
                   ? 3
                   : (v>=0x04u)
                     ? 2
                     : (v>=0x02u)
                       ? 1
                       : 0
           ;
}

constexpr inline int getMsbPower(uint16_t v) { return getMsbPower(uint8_t (v>> 8)) ? getMsbPower(uint8_t (v>> 8))+  8 : getMsbPower(uint8_t (v)); }
constexpr inline int getMsbPower(uint32_t v) { return getMsbPower(uint16_t(v>>16)) ? getMsbPower(uint16_t(v>>16))+ 16 : getMsbPower(uint16_t(v)); }
constexpr inline int getMsbPower(uint64_t v) { return getMsbPower(uint32_t(v>>32)) ? getMsbPower(uint32_t(v>>32))+ 32 : getMsbPower(uint32_t(v)); }

constexpr inline int getMsbPower(int8_t  v)  { return getMsbPower(uint8_t (v)); }
constexpr inline int getMsbPower(int16_t v)  { return getMsbPower(uint16_t(v)); }
constexpr inline int getMsbPower(int32_t v)  { return getMsbPower(uint32_t(v)); }
constexpr inline int getMsbPower(int64_t v)  { return getMsbPower(uint64_t(v)); }

//----------------------------------------------------------------------------
inline
uint64_t makeByteSizeMask(int n)
{
    switch(n)
    {
        case 1 : return std::uint64_t(0x00000000000000FFull);
        case 2 : return std::uint64_t(0x000000000000FFFFull);
        case 3 : return std::uint64_t(0x0000000000FFFFFFull);
        case 4 : return std::uint64_t(0x00000000FFFFFFFFull);
        case 5 : return std::uint64_t(0x000000FFFFFFFFFFull);
        case 6 : return std::uint64_t(0x0000FFFFFFFFFFFFull);
        case 7 : return std::uint64_t(0x00FFFFFFFFFFFFFFull);
        default: return std::uint64_t(0xFFFFFFFFFFFFFFFFull);
    }
}

//----------------------------------------------------------------------------
template<int NumBits> constexpr uint64_t makeMask() { return 0; }

template<> constexpr uint64_t makeMask< 0>() { return 0x0000000000000000ull; }

template<> constexpr uint64_t makeMask< 1>() { return 0x0000000000000001ull; }
template<> constexpr uint64_t makeMask< 2>() { return 0x0000000000000003ull; }
template<> constexpr uint64_t makeMask< 3>() { return 0x0000000000000007ull; }
template<> constexpr uint64_t makeMask< 4>() { return 0x000000000000000Full; }

template<> constexpr uint64_t makeMask< 5>() { return 0x000000000000001Full; }
template<> constexpr uint64_t makeMask< 6>() { return 0x000000000000003Full; }
template<> constexpr uint64_t makeMask< 7>() { return 0x000000000000007Full; }
template<> constexpr uint64_t makeMask< 8>() { return 0x00000000000000FFull; }

template<> constexpr uint64_t makeMask< 9>() { return 0x00000000000001FFull; }
template<> constexpr uint64_t makeMask<10>() { return 0x00000000000003FFull; }
template<> constexpr uint64_t makeMask<11>() { return 0x00000000000007FFull; }
template<> constexpr uint64_t makeMask<12>() { return 0x0000000000000FFFull; }

template<> constexpr uint64_t makeMask<13>() { return 0x0000000000001FFFull; }
template<> constexpr uint64_t makeMask<14>() { return 0x0000000000003FFFull; }
template<> constexpr uint64_t makeMask<15>() { return 0x0000000000007FFFull; }
template<> constexpr uint64_t makeMask<16>() { return 0x000000000000FFFFull; }

template<> constexpr uint64_t makeMask<17>() { return 0x000000000001FFFFull; }
template<> constexpr uint64_t makeMask<18>() { return 0x000000000003FFFFull; }
template<> constexpr uint64_t makeMask<19>() { return 0x000000000007FFFFull; }
template<> constexpr uint64_t makeMask<20>() { return 0x00000000000FFFFFull; }

template<> constexpr uint64_t makeMask<21>() { return 0x00000000001FFFFFull; }
template<> constexpr uint64_t makeMask<22>() { return 0x00000000003FFFFFull; }
template<> constexpr uint64_t makeMask<23>() { return 0x00000000007FFFFFull; }
template<> constexpr uint64_t makeMask<24>() { return 0x0000000000FFFFFFull; }

template<> constexpr uint64_t makeMask<25>() { return 0x0000000001FFFFFFull; }
template<> constexpr uint64_t makeMask<26>() { return 0x0000000003FFFFFFull; }
template<> constexpr uint64_t makeMask<27>() { return 0x0000000007FFFFFFull; }
template<> constexpr uint64_t makeMask<28>() { return 0x000000000FFFFFFFull; }

template<> constexpr uint64_t makeMask<29>() { return 0x000000001FFFFFFFull; }
template<> constexpr uint64_t makeMask<30>() { return 0x000000003FFFFFFFull; }
template<> constexpr uint64_t makeMask<31>() { return 0x000000007FFFFFFFull; }
template<> constexpr uint64_t makeMask<32>() { return 0x00000000FFFFFFFFull; }

template<> constexpr uint64_t makeMask<33>() { return 0x00000001FFFFFFFFull; }
template<> constexpr uint64_t makeMask<34>() { return 0x00000003FFFFFFFFull; }
template<> constexpr uint64_t makeMask<35>() { return 0x00000007FFFFFFFFull; }
template<> constexpr uint64_t makeMask<36>() { return 0x0000000FFFFFFFFFull; }

template<> constexpr uint64_t makeMask<37>() { return 0x0000001FFFFFFFFFull; }
template<> constexpr uint64_t makeMask<38>() { return 0x0000003FFFFFFFFFull; }
template<> constexpr uint64_t makeMask<39>() { return 0x0000007FFFFFFFFFull; }
template<> constexpr uint64_t makeMask<40>() { return 0x000000FFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<41>() { return 0x000001FFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<42>() { return 0x000003FFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<43>() { return 0x000007FFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<44>() { return 0x00000FFFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<45>() { return 0x00001FFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<46>() { return 0x00003FFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<47>() { return 0x00007FFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<48>() { return 0x0000FFFFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<49>() { return 0x0001FFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<50>() { return 0x0003FFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<51>() { return 0x0007FFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<52>() { return 0x000FFFFFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<53>() { return 0x001FFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<54>() { return 0x003FFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<55>() { return 0x007FFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<56>() { return 0x00FFFFFFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<57>() { return 0x01FFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<58>() { return 0x03FFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<59>() { return 0x07FFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<60>() { return 0x0FFFFFFFFFFFFFFFull; }

template<> constexpr uint64_t makeMask<61>() { return 0x1FFFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<62>() { return 0x3FFFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<63>() { return 0x7FFFFFFFFFFFFFFFull; }
template<> constexpr uint64_t makeMask<64>() { return 0xFFFFFFFFFFFFFFFFull; }


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
constexpr uint64_t makeMask(int nBits)
{
    return (nBits==0)
           ? makeMask<0>()
           : (nBits==1)
             ? makeMask<1>()
             : (nBits==2)
               ? makeMask<2>()
               : (nBits==3)
                 ? makeMask<3>()
                 : (nBits==4)
                   ? makeMask<4>()
                   : (nBits==5)
                     ? makeMask<5>()
                     : (nBits==6)
                       ? makeMask<6>()
                       : (nBits==7)
                         ? makeMask<7>()
                         : (nBits==8)
                           ? makeMask<8>()
                           : (nBits==9)
                             ? makeMask<9>()
                             : (nBits==10)
                               ? makeMask<10>()
                               : (nBits==11)
                                 ? makeMask<11>()
                                 : (nBits==12)
                                   ? makeMask<12>()
                                   : (nBits==13)
                                     ? makeMask<13>()
                                     : (nBits==14)
                                       ? makeMask<14>()
                                       : (nBits==15)
                                         ? makeMask<15>()
                                         : (nBits==16)
                                           ? makeMask<16>()
                                           : (nBits==17)
                                             ? makeMask<17>()
                                             : (nBits==18)
                                               ? makeMask<18>()
                                               : (nBits==19)
                                                 ? makeMask<19>()
                                                 : (nBits==20)
                                                   ? makeMask<20>()
                                                   : (nBits==21)
                                                     ? makeMask<21>()
                                                     : (nBits==22)
                                                       ? makeMask<22>()
                                                       : (nBits==23)
                                                         ? makeMask<23>()
                                                         : (nBits==24)
                                                           ? makeMask<24>()
                                                           : (nBits==25)
                                                             ? makeMask<25>()
                                                             : (nBits==26)
                                                               ? makeMask<26>()
                                                               : (nBits==27)
                                                                 ? makeMask<27>()
                                                                 : (nBits==28)
                                                                   ? makeMask<28>()
                                                                   : (nBits==29)
                                                                     ? makeMask<29>()
                                                                     : (nBits==30)
                                                                       ? makeMask<30>()
                                                                       : (nBits==31)
                                                                         ? makeMask<31>()
                                                                         : (nBits==32)
                                                                           ? makeMask<32>()
                                                                           : (nBits==33)
                                                                             ? makeMask<33>()
                                                                             : (nBits==34)
                                                                               ? makeMask<34>()
                                                                               : (nBits==35)
                                                                                 ? makeMask<35>()
                                                                                 : (nBits==36)
                                                                                   ? makeMask<36>()
                                                                                   : (nBits==37)
                                                                                     ? makeMask<37>()
                                                                                     : (nBits==38)
                                                                                       ? makeMask<38>()
                                                                                       : (nBits==39)
                                                                                         ? makeMask<39>()
                                                                                         : (nBits==40)
                                                                                           ? makeMask<40>()
                                                                                           : (nBits==41)
                                                                                             ? makeMask<41>()
                                                                                             : (nBits==42)
                                                                                               ? makeMask<42>()
                                                                                               : (nBits==43)
                                                                                                 ? makeMask<43>()
                                                                                                 : (nBits==44)
                                                                                                   ? makeMask<44>()
                                                                                                   : (nBits==45)
                                                                                                     ? makeMask<45>()
                                                                                                     : (nBits==46)
                                                                                                       ? makeMask<46>()
                                                                                                       : (nBits==47)
                                                                                                         ? makeMask<47>()
                                                                                                         : (nBits==48)
                                                                                                           ? makeMask<48>()
                                                                                                           : (nBits==49)
                                                                                                             ? makeMask<49>()
                                                                                                             : (nBits==50)
                                                                                                               ? makeMask<50>()
                                                                                                               : (nBits==51)
                                                                                                                 ? makeMask<51>()
                                                                                                                 : (nBits==52)
                                                                                                                   ? makeMask<52>()
                                                                                                                   : (nBits==53)
                                                                                                                     ? makeMask<53>()
                                                                                                                     : (nBits==54)
                                                                                                                       ? makeMask<54>()
                                                                                                                       : (nBits==55)
                                                                                                                         ? makeMask<55>()
                                                                                                                         : (nBits==56)
                                                                                                                           ? makeMask<56>()
                                                                                                                           : (nBits==57)
                                                                                                                             ? makeMask<57>()
                                                                                                                             : (nBits==58)
                                                                                                                               ? makeMask<58>()
                                                                                                                               : (nBits==59)
                                                                                                                                 ? makeMask<59>()
                                                                                                                                 : (nBits==60)
                                                                                                                                   ? makeMask<60>()
                                                                                                                                   : (nBits==61)
                                                                                                                                     ? makeMask<61>()
                                                                                                                                     : (nBits==62)
                                                                                                                                       ? makeMask<62>()
                                                                                                                                       : (nBits==63)
                                                                                                                                         ? makeMask<63>()
                                                                                                                                         : makeMask<64>()
           ;

    // uint610_t res = 0u;
    // uint610_t bit = 1u;
    //  
    // for(auto i=0; i!=nBits; ++i, bit<<=1)
    //     res |= bit;
    //  
    // return res;
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------


} // namespace bits
} // namespace mem
} // namespace marty
// marty::mem::bits::
// #include "marty_mem/bits.h"
