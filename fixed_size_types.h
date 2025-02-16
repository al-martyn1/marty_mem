/*! \file
    \brief Алиасы для целых типов
 */

#pragma once

//----------------------------------------------------------------------------
/*
    Типы std::intXX_t/std::uintXX_t обычно объявляются из сишных типов 
    intXX_t/uintXX_t путем включения сишных хидеров внутри пространства имён std.
    Этим иногда пользуются, и не указывают при использовании пространство имён std.
    Но, по идее, так делать неправильно.
    Поэтому мы явно вводим эти типы в наше пространство имён.
 */


//----------------------------------------------------------------------------
// #include "marty_mem/fixed_size_types.h"
// marty::mem::
namespace marty{
namespace mem{

#include "fixed_size_types_.h"

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/fixed_size_types.h"

