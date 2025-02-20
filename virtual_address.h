/*! \file
    \brief Виртуальный адрес - интерфейс
 */

/*
    Если мы такой адрес будем использовать в итераторе, а там всякие инкременты/декременты пре/пост,
    то нам придётся часто клонировать объекты - наследники VirtualAddress. В этом есть некоторая проблема,
    но ничего особо не сделать.

*/

#pragma once

//----------------------------------------------------------------------------
#include "assert.h"
#include "bits.h"
#include "fixed_size_types.h"

//----------------------------------------------------------------------------
#include <exception>
#include <memory>
#include <stdexcept>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/marty_mem.h"
// marty::mem::
namespace marty{
namespace mem{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
struct VirtualAddress;
using SharedVirtualAddress = std::shared_ptr<VirtualAddress>;


//----------------------------------------------------------------------------
struct VirtualAddress
{

    virtual ~VirtualAddress() {}
    virtual SharedVirtualAddress clone() = 0;
    virtual void setIncrement(uint64_t) = 0;
    virtual void inc() = 0;
    virtual void dec() = 0;
    virtual void add(ptrdiff_t d) = 0;
    virtual void subtract(ptrdiff_t d) = 0;
    virtual uint64_t getLinearAddress() = 0;
    virtual std::string toString() = 0;
    virtual ptrdiff_t distanceTo(const VirtualAddress *pv) = 0;  // "Расстояние" от текущего до pv - сколько надо прибавить к текущему, чтобы получить pv => *pv > *this => dist = pv - dist
    virtual bool equalTo(const VirtualAddress *pv) = 0;

}; // struct VirtualAddress

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

