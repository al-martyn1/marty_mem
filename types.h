/*! \file
    \brief Определения базовых типов
 */

#pragma once

//----------------------------------------------------------------------------
#include "fixed_size_types.h"

//----------------------------------------------------------------------------
#include <string>
#include <vector>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/types.h"
// marty::mem::
namespace marty{
namespace mem{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if !defined(MARTY_MEM_BYTE_VECTOR_TYPE)

    #if defined(DEBUG) || defined(_DEBUG)

        // std::vector is very cool for debugging
        #define MARTY_MEM_BYTE_VECTOR_TYPE    std::vector<byte_t>

    #else

        // std::basic_string uses small string optimization and faster on short byte vectors
        #define MARTY_MEM_BYTE_VECTOR_TYPE    std::basic_string<byte_t>

    #endif

#endif

//----------------------------------------------------------------------------
// TODO: learn https://en.cppreference.com/w/cpp/language/template_parameters

using byte_vector_t = MARTY_MEM_BYTE_VECTOR_TYPE;

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/types.h"


