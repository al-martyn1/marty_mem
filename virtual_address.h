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

struct AddressInfo
{
   uint64_t    base  ;
   uint64_t    offset;

}; // struct AddressInfo


//----------------------------------------------------------------------------
struct VirtualAddress
{

    virtual ~VirtualAddress() {}
    virtual SharedVirtualAddress clone() const = 0;
    virtual void setIncrement(uint64_t) = 0;
    virtual bool inc() = 0;                      // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool dec() = 0;                      // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool add(ptrdiff_t d) = 0;           // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool subtract(ptrdiff_t d) = 0;      // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual uint64_t getLinearAddress() const = 0;
    virtual std::string toString() const = 0;
    virtual ptrdiff_t distanceTo(const VirtualAddress *pv) const = 0;  // "Расстояние" от текущего до pv - сколько надо прибавить к текущему, чтобы получить pv => *pv > *this => dist = pv - dist
    virtual bool equalTo(const VirtualAddress *pv) const = 0;
    virtual AddressInfo getAddressInfo() const = 0;
    virtual bool checkAddressInValidSizeRange() const = 0;


}; // struct VirtualAddress

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

