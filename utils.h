/*! \file
    \brief Утилиты для marty::mem
 */

#pragma once

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#include "assert.h"
#include "fixed_size_types.h"
#include "types.h"

//------------------------------
#include <algorithm>
#include <string>
#include <unordered_map>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if !defined(MARTY_USED)

    #if defined(UMBA_USED)

        #define MARTY_USED(x)    UMBA_USED(x)

    #else
    
        #define MARTY_USED(x)    (void)(x)

    #endif

#endif

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/utils.h"
// marty::mem::utils::
namespace marty{
namespace mem{
namespace utils{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename CharType>
CharType digitToHexChar(int d, bool bLower=false)
{
    d &= 0xF;
    // if (d<0)  return '-';
    if (d<10) return char(CharType('0')+d);
    return char((bLower?CharType('a'):CharType('A'))+d-10);
}

//----------------------------------------------------------------------------
template<typename CharType>
int hexCharToDigit(CharType ch)
{
    if (ch>=CharType('0') && ch<=CharType('9'))
        return ch-CharType('0');

    if (ch>=CharType('A') && ch<=CharType('F'))
        return ch-CharType('A')+10;

    if (ch>=CharType('a') && ch<=CharType('f'))
        return ch-CharType('a')+10;

    return -1;
}

//----------------------------------------------------------------------------
template<typename StringType=std::string>
StringType makeHexString(uint64_t val, std::size_t size)
{
    MARTY_MEM_ASSERT(size>=1u && size<=8u);

    using CharType = typename StringType::value_type;

    StringType resStr; resStr.reserve(16);
    for(auto i=0u; i!=16u; ++i, val>>=4)
    {
        resStr.append(1, digitToHexChar<CharType>((int)unsigned(val&0xF)));
    }

    // Старшие - у нас в конце
    resStr.erase(size*2);
    std::reverse(resStr.begin(), resStr.end());

    return resStr;
}

//----------------------------------------------------------------------------
template<typename StringType=std::string>
StringType stringExtend(StringType str, std::size_t size)
{
    // std::size_t fillSize = 0;

    if (str.size()<size)
    {
        str.append(size-str.size(), typename StringType::value_type(' '));
    }

    return str;
}


//----------------------------------------------------------------------------
template<typename StringType=std::string> StringType makeHexString(uint64_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString( int64_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString(uint32_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString( int32_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString(uint16_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString( int16_t val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString(uint8_t  val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }
template<typename StringType=std::string> StringType makeHexString( int8_t  val) { return makeHexString<StringType>(uint64_t(val), sizeof(val)); }

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace utils
} // namespace mem
} // namespace marty
// marty::mem::utils::
// #include "marty_mem/utils.h"
