/*! \file
    \brief Реализация линейного адреса
 */

#pragma once

//----------------------------------------------------------------------------
#include "assert.h"
#include "bits.h"
#include "fixed_size_types.h"

//----------------------------------------------------------------------------
#include <exception>
#include <stdexcept>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/marty_mem.h"
// marty::mem::
namespace marty{
namespace mem{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
struct LinearAddressTraits
{

}; // struct LinearAddressTraits

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class LinearAddress
{
    uint64_t                 m_address     = 0;
    uint64_t                 m_incSize     = 1;

    void inc()
    {
        m_address += m_incSize;
    }

    void dec()
    {
        m_address -= m_incSize;
    }


public:

    static bool checkTraits(const LinearAddressTraits &traits)
    {
        return true;
    }

    static bool checkIncrement(uint64_t incSize)
    {
        return bits::countOnes(incSize)==1 && incSize<=8; // Не поддерживается гранулярность обращения к памяти больше 8 байт
    }

    uint64_t getLinearAddress() const
    {
        return m_address;
    }

    LinearAddress() {}

    LinearAddress(uint64_t addr, uint64_t inc=1, const LinearAddressTraits &traits=LinearAddressTraits{})
    : m_address(addr)
    , m_incSize(inc)
    , m_traits (traits)
    {
        MARTY_MEM_ASSERT(checkTraits(m_traits));
        MARTY_MEM_ASSERT(checkIncrement(incSize));
    }


    // При сравнении адресов размер инкремента нам не интересен
    bool operator==(const LinearAddress &other) const
    {
        return m_address==other.m_address;
    }

    bool operator!=(const LinearAddress &other) const
    {
        return m_address!=other.m_address;
    }

    LinearAddress& operator++() // pre
    {
        inc();
        return *this;
    }
     
    LinearAddress operator++(int) // post
    {
        auto cp = *this;
        inc();
        return cp;
    }

    LinearAddress& operator--() // pre
    {
        dec();
        return *this;
    }
     
    LinearAddress operator--(int) // post
    {
        auto cp = *this;
        dec();
        return cp;
    }

}; // struct LinearAddress

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"
