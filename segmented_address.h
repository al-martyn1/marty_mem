/*! \file
    \brief Реализация сегментного адреса
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
struct SegmentedAddressTraits
{
    uint64_t segmentBitSize = 16u;
    uint64_t offsetBitSize  = 16u;
    uint64_t paragraphSize  = 16u;

}; // struct SegmentedAddressTraits

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class SegmentedAddress
{
    uint64_t                 m_segment     = 0;
    uint64_t                 m_offset      = 0;
    uint64_t                 m_incSize     = 1;
    SegmentedAddressTraits   m_traits         ;
    uint64_t                 m_segmentMask = 0;
    uint64_t                 m_offsetMask  = 0;

    void inc()
    {
        m_offset += m_incSize;
        m_offset &= m_offsetMask;
    }

    void dec()
    {
        m_offset -= m_incSize;
        m_offset &= m_offsetMask;
    }


public:

    static bool checkTraits(const SegmentedAddressTraits &traits)
    {
        return bits::countOnes(traits.paragraphSize)==1;
    }

    static bool checkIncrement(uint64_t incSize)
    {
        return bits::countOnes(incSize)==1 && incSize<=8; // Не поддерживается гранулярность обращения к памяти больше 8 байт
    }

    uint64_t getLinearAddress() const
    {
        return (m_segment&m_segmentMask) + (m_offset&m_offsetMask);
    }

    SegmentedAddress() {}

    SegmentedAddress(uint64_t seg, uint64_t offs, uint64_t inc=1, const SegmentedAddressTraits &traits=SegmentedAddressTraits{})
    : m_segment(seg)
    , m_offset (offs)
    , m_incSize(inc)
    , m_traits (traits)
    , m_segmentMask(bits::makeMask(m_traits.segmentBitSize))
    , m_offsetMask (bits::makeMask(m_traits.offsetBitSize))
    {
        MARTY_MEM_ASSERT(checkTraits(m_traits));
        MARTY_MEM_ASSERT(checkIncrement(incSize));
    }


    // При сравнении адресов размер инкремента нам не интересен
    bool operator==(const SegmentedAddress &other) const
    {
        return m_segment==other.m_segment && m_offset==other.m_offset && m_traits==other.m_traits;
    }

    bool operator!=(const SegmentedAddress &other) const
    {
        return m_segment!=other.m_segment || m_offset!=other.m_offset || m_traits!=other.m_traits;
    }

    SegmentedAddress& operator++() // pre
    {
        inc();
        return *this;
    }
     
    SegmentedAddress operator++(int) // post
    {
        auto cp = *this;
        inc();
        return cp;
    }

    SegmentedAddress& operator--() // pre
    {
        dec();
        return *this;
    }
     
    SegmentedAddress operator--(int) // post
    {
        auto cp = *this;
        dec();
        return cp;
    }

}; // struct SegmentedAddress

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"
