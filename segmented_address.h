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
    uint64_t                 m_segment      = 0;
    uint64_t                 m_offset       = 0;
    uint64_t                 m_incSize      = 1;
    SegmentedAddressTraits   m_traits          ;
    uint64_t                 m_segmentMask  = 0;
    uint64_t                 m_offsetMask   = 0;
    unsigned                 m_segmentShift = 4;

    

    // See also: A look back at memory models in 16-bit MS-DOS - https://devblogs.microsoft.com/oldnewthing/20200728-00/?p=104012
    // See also: On memory allocations larger than 64KB on 16-bit Windows - https://devblogs.microsoft.com/oldnewthing/20171113-00/?p=97386
    // See also: Revisiting the DOS memory models - https://blogsystem5.substack.com/p/dos-memory-models

    // Conclusion: When the offset wraps around, you add 0x1000 to the segment. 
    // In MS-DOS, huge pointers operated by putting the upper 16 bits of the 20-bit address in the segment and putting the remaining 4 bits in the offset. The offset of a huge pointer in MS-DOS was always less than 16.
    // В MS-DOS огромные указатели работали, помещая верхние 16 бит 20-битного адреса в сегмент, а оставшиеся 4 бита — в смещение. Смещение огромного указателя в MS-DOS всегда было меньше 16.
    // The __AHINCR variable is a variable exported from KERNEL. In real mode Windows, the value is 0x1000. In protected mode Windows, the value is 0x0008. When your program reaches the end of a 64KB block, it uses the __AHINCR value to decide how much to increment the segment/selector by in order to reach the next 64KB block. 

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
        return ((m_segment&m_segmentMask)<<m_segmentShift) + (m_offset&m_offsetMask);
    }

    SegmentedAddress() {}

    SegmentedAddress(uint64_t seg, uint64_t offs, uint64_t inc=1, const SegmentedAddressTraits &traits=SegmentedAddressTraits{})
    : m_segment(seg)
    , m_offset (offs)
    , m_incSize(inc)
    , m_traits (traits)
    , m_segmentMask (bits::makeMask(traits.segmentBitSize))
    , m_offsetMask  (bits::makeMask(traits.offsetBitSize))
    , m_segmentShift(bits::getMsbPower(traits.paragraphSize))
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
