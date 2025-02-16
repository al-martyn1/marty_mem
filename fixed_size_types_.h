/*! \file
    \brief Алиасы для целых типов. Версия для включения в произвольное пространство имён
 */

//----------------------------------------------------------------------------
/*
    Типы std::intXX_t/std::uintXX_t обычно объявляются из сишных типов 
    intXX_t/uintXX_t путем включения сишных хидеров внутри пространства имён std.
    Этим иногда пользуются, и не указывают при использовании пространство имён std.
    Но, по идее, так делать неправильно.
    Поэтому мы явно вводим эти типы в наше пространство имён включением данного файла
    в требуемом скоупе.

 */


#include <cstdint>
#include <string>


using int8_t     = std::int8_t  ;
using int16_t    = std::int16_t ;
using int32_t    = std::int32_t ;
using int64_t    = std::int64_t ;

using uint8_t    = std::uint8_t ;
using uint16_t   = std::uint16_t;
using uint32_t   = std::uint32_t;
using uint64_t   = std::uint64_t;

using byte_t     = uint8_t; // Наш собственный алиас


template<typename IntType>
const char* getFixedSizeTypeName()
{
    return "unknown_t";
}

template<> const char* getFixedSizeTypeName<int8_t  >() { return "int8_t" ; }
template<> const char* getFixedSizeTypeName<int16_t >() { return "int16_t"; }
template<> const char* getFixedSizeTypeName<int32_t >() { return "int32_t"; }
template<> const char* getFixedSizeTypeName<int64_t >() { return "int64_t"; }

template<> const char* getFixedSizeTypeName<uint8_t >() { return "uint8_t" ; }
template<> const char* getFixedSizeTypeName<uint16_t>() { return "uint16_t"; }
template<> const char* getFixedSizeTypeName<uint32_t>() { return "uint32_t"; }
template<> const char* getFixedSizeTypeName<uint64_t>() { return "uint64_t"; }

