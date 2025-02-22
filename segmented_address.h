/*! \file
    \brief Реализация сегментного адреса
 */

#pragma once

//----------------------------------------------------------------------------
#include "assert.h"
#include "bits.h"
#include "fixed_size_types.h"
#include "virtual_address.h"
#include "utils.h"

//----------------------------------------------------------------------------
#include <exception>
#include <memory>
#include <stdexcept>
#include <typeinfo>

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

inline bool operator==(const SegmentedAddressTraits &t1, const SegmentedAddressTraits &t2)
{
    return t1.segmentBitSize==t2.segmentBitSize && t1.offsetBitSize==t2.offsetBitSize && t1.paragraphSize==t2.paragraphSize;
}

inline bool operator!=(const SegmentedAddressTraits &t1, const SegmentedAddressTraits &t2)
{
    return t1.segmentBitSize!=t2.segmentBitSize || t1.offsetBitSize!=t2.offsetBitSize || t1.paragraphSize!=t2.paragraphSize;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class SegmentedAddress : public VirtualAddress
{
    uint64_t                 m_segment      = 0;
    uint64_t                 m_offset       = 0;
    uint64_t                 m_incSize      = 1;
    SegmentedAddressTraits   m_traits          ;
    uint64_t                 m_segmentMask  = 0;
    uint64_t                 m_offsetMask   = 0;
    int                      m_segmentShift = 4;

    

    // See also: A look back at memory models in 16-bit MS-DOS - https://devblogs.microsoft.com/oldnewthing/20200728-00/?p=104012
    // See also: On memory allocations larger than 64KB on 16-bit Windows - https://devblogs.microsoft.com/oldnewthing/20171113-00/?p=97386
    // See also: Revisiting the DOS memory models - https://blogsystem5.substack.com/p/dos-memory-models

    // Conclusion: When the offset wraps around, you add 0x1000 to the segment. 
    // In MS-DOS, huge pointers operated by putting the upper 16 bits of the 20-bit address in the segment and putting the remaining 4 bits in the offset. The offset of a huge pointer in MS-DOS was always less than 16.
    // В MS-DOS огромные указатели работали, помещая верхние 16 бит 20-битного адреса в сегмент, а оставшиеся 4 бита — в смещение. Смещение огромного указателя в MS-DOS всегда было меньше 16.
    // The __AHINCR variable is a variable exported from KERNEL. In real mode Windows, the value is 0x1000. In protected mode Windows, the value is 0x0008. When your program reaches the end of a 64KB block, it uses the __AHINCR value to decide how much to increment the segment/selector by in order to reach the next 64KB block. 


public:


    virtual void setIncrement(uint64_t v) override
    {
        m_incSize = v;
        MARTY_MEM_ASSERT(checkIncrement(m_incSize));
    }

        // auto offsSaved = m_offset;
        // m_address &= m_addressMask;
        // return addrSaved!=m_address;

    virtual bool inc() override
    {
        m_offset += m_incSize;
        auto offsSaved = m_offset;
        m_offset &= m_offsetMask;
        return offsSaved!=m_offset;
    }

    virtual bool dec() override
    {
        m_offset -= m_incSize;
        auto offsSaved = m_offset;
        m_offset &= m_offsetMask;
        return offsSaved!=m_offset;
    }

    virtual bool add(ptrdiff_t d) override
    {
        m_offset += uint64_t(d*m_incSize);
        auto offsSaved = m_offset;
        m_offset &= m_offsetMask;
        return offsSaved!=m_offset;
    }

    virtual bool subtract(ptrdiff_t d) override
    {
        m_offset -= uint64_t(d*m_incSize);
        auto offsSaved = m_offset;
        m_offset &= m_offsetMask;
        return offsSaved!=m_offset;
    }

    virtual uint64_t getLinearAddress() override
    {
        return ((m_segment&m_segmentMask)<<m_segmentShift) + (m_offset&m_offsetMask);
    }

    virtual std::string toString() override
    {
        auto numDigitsSeg = m_traits.segmentBitSize/4;
        if (m_traits.segmentBitSize%4)
           ++numDigitsSeg;
        if (numDigitsSeg%2)
           ++numDigitsSeg;

        auto numDigitsOffs = m_traits.offsetBitSize/4;
        if (m_traits.offsetBitSize%4)
           ++numDigitsOffs;
        if (numDigitsOffs%2)
           ++numDigitsOffs;

        return utils::makeHexString<std::string>(m_segment, std::size_t(numDigitsSeg/2)) + ":" + utils::makeHexString<std::string>(m_offset, std::size_t(numDigitsOffs/2)); // pass num bytes
    }

    void checkCompat(const SegmentedAddress &other) const
    {
        // MARTY_MEM_ASSERT(m_incSize==other.m_incSize); // Разные размерности недопустимы
        // MARTY_MEM_ASSERT(m_traits==other.m_traits);
        if (m_incSize!=other.m_incSize)
            throw incompatible_address_pointers("incompatible address pointers: addressed value size is different between two pointers");
        if (m_traits!=other.m_traits)
            throw incompatible_address_pointers("incompatible address pointers: pointer traits is different between two pointers");
    }
    
    void checkDiff(uint64_t diff, const char *msg)
    {
        int64_t diffMod = int64_t(diff)<0 ? -int64_t(diff) : int64_t(diff);
        auto sizeofIntType1 = m_incSize - 1u;
        auto mask = bits::makeMask(int(sizeofIntType1));
        auto diffNmask = diffMod&mask;
        if (diffNmask!=0)
            throw invalid_address_difference(msg);
    }

    // "Расстояние" от текущего до pv - сколько надо прибавить к текущему, чтобы получить pv => *pv > *this => dist = pv - dist
    virtual ptrdiff_t distanceTo(const VirtualAddress *pv) override
    {
        const SegmentedAddress &other = dynamic_cast<const SegmentedAddress&>(*pv); // Чтобы самим не кидать исключение bad_cast, используем ссылки
        checkCompat(other);
        auto diff = other.m_offset - m_offset;
        diff &= m_offsetMask;
        checkDiff(diff, "the difference in addresses is not a multiple of the type size");
        return ptrdiff_t(diff) / ptrdiff_t(m_incSize);
    }

    virtual bool equalTo(const VirtualAddress *pv) override
    {
        const SegmentedAddress &other = dynamic_cast<const SegmentedAddress&>(*pv); // Чтобы самим не кидать исключение bad_cast, используем ссылки
        checkCompat(other);
        auto diff = other.m_offset - m_offset;
        diff &= m_offsetMask;
        checkDiff(diff, "the difference in addresses is not a multiple of the type size");
        return m_segment==other.m_segment && m_offset==other.m_offset;
    }

    virtual SharedVirtualAddress clone() override
    {
        auto copyOfThis = std::make_shared<SegmentedAddress>(*this);
        return std::static_pointer_cast<VirtualAddress>(copyOfThis);
    }

    static bool checkTraits(const SegmentedAddressTraits &traits)
    {
        return bits::countOnes(traits.paragraphSize)==1;
    }

    static bool checkIncrement(uint64_t incSize)
    {
        return bits::countOnes(incSize)==1 && incSize<=8; // Не поддерживается гранулярность обращения к памяти больше 8 байт
    }

    SegmentedAddress() {}

    SegmentedAddress(uint64_t seg, uint64_t offs, uint64_t inc=1, const SegmentedAddressTraits &traits=SegmentedAddressTraits{})
    : m_segment(seg)
    , m_offset (offs)
    , m_incSize(inc)
    , m_traits (traits)
    , m_segmentMask (bits::makeMask(int(traits.segmentBitSize)))
    , m_offsetMask  (bits::makeMask(int(traits.offsetBitSize)))
    , m_segmentShift(bits::getMsbPower(traits.paragraphSize))
    {
        MARTY_MEM_ASSERT(checkTraits(m_traits));
        MARTY_MEM_ASSERT(checkIncrement(m_incSize));
    }


    // При сравнении адресов размер инкремента нам не интересен
    bool operator==(const SegmentedAddress &other) const
    {
        MARTY_MEM_ASSERT(m_traits==other.m_traits);
        return m_segment==other.m_segment && m_offset==other.m_offset;
    }

    bool operator!=(const SegmentedAddress &other) const
    {
        MARTY_MEM_ASSERT(m_traits==other.m_traits);
        return m_segment!=other.m_segment || m_offset!=other.m_offset;
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

