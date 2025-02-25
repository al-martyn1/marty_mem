/*! \file
    \brief Реализация линейного адреса
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
struct LinearAddressTraits
{
    uint64_t addressBitSize     = 32u;
    uint64_t extraWrapBitSize   =  0u;

}; // struct LinearAddressTraits

inline bool operator==(const LinearAddressTraits &t1, const LinearAddressTraits &t2)
{
    return t1.addressBitSize==t2.addressBitSize;
}

inline bool operator!=(const LinearAddressTraits &t1, const LinearAddressTraits &t2)
{
    return t1.addressBitSize!=t2.addressBitSize;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class LinearAddress : public VirtualAddress
{
    uint64_t                 m_address       = 0;
    uint64_t                 m_incSize       = 1;
    LinearAddressTraits      m_traits        = LinearAddressTraits();
    uint64_t                 m_addressMask   = 0;
    uint64_t                 m_extraWrapMask = 0;

    // Возвращает true, если по доп маске не было переполнения
    bool checkExtraWrap(uint64_t addrPrev) const
    {
        if (m_traits.extraWrapBitSize==0) return true;

        uint64_t addrMasked = m_address & ~m_extraWrapMask;
        addrPrev &= ~m_extraWrapMask;

        return addrPrev==addrMasked;
    }

public:

    virtual bool checkAddressInValidSizeRange() const override
    {
        uint64_t m_addressTmp = m_address;
        m_addressTmp &= m_addressMask;

        return m_addressTmp==m_address;
    }

    virtual AddressInfo getAddressInfo() const override
    {
        AddressInfo ai;
        ai.base   = m_address;
        ai.offset = 0;
        return ai;
    }

    virtual void setIncrement(uint64_t v) override
    {
        m_incSize = v;
        MARTY_MEM_ASSERT(checkIncrement(m_incSize));
    }

    // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool inc() override
    {
        m_address += m_incSize;
        auto addrSaved = m_address;
        m_address &= m_addressMask;
        return addrSaved!=m_address && !checkExtraWrap(addrSaved);
    }

    // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool dec() override
    {
        m_address -= m_incSize;
        auto addrSaved = m_address;
        m_address &= m_addressMask;
        return addrSaved!=m_address && !checkExtraWrap(addrSaved);
    }

    // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool add(ptrdiff_t d) override
    {
        m_address += uint64_t(d*m_incSize);
        auto addrSaved = m_address;
        m_address &= m_addressMask;
        return addrSaved!=m_address && !checkExtraWrap(addrSaved);
    }

    // Возвращает true, если было переполнение адреса/смещения в сегменте
    virtual bool subtract(ptrdiff_t d) override
    {
        m_address -= uint64_t(d*m_incSize);
        auto addrSaved = m_address;
        m_address &= m_addressMask;
        return addrSaved!=m_address && !checkExtraWrap(addrSaved);
    }

    virtual uint64_t getLinearAddress() const override
    {
        return m_address;
    }

    virtual std::string toString() const override
    {
        auto numDigits = m_traits.addressBitSize/4;
        if (m_traits.addressBitSize%4)
           ++numDigits;
        if (numDigits%2)
           ++numDigits;

        return utils::makeHexString<std::string>(m_address, std::size_t(numDigits/2)); // pass num bytes
    }

    void checkCompat(const LinearAddress &other) const
    {
        // MARTY_MEM_ASSERT(m_incSize==other.m_incSize); // Разные размерности недопустимы
        // MARTY_MEM_ASSERT(m_traits==other.m_traits);
        if (m_incSize!=other.m_incSize)
            throw incompatible_address_pointers("incompatible address pointers: addressed value size is different between two pointers");
        // if (m_traits!=other.m_traits)
        //     throw incompatible_address_pointers("incompatible address pointers: pointer traits is different between two pointers");
    }

    void checkDiff(uint64_t diff, const char *msg) const
    {
        int64_t diffMod = int64_t(diff)<0 ? -int64_t(diff) : int64_t(diff);
        auto sizeofIntType1 = m_incSize - 1u;
        auto mask = bits::makeMask(int(sizeofIntType1));
        auto diffNmask = diffMod&mask;
        if (diffNmask!=0)
            throw invalid_address_difference(msg);
    }

    // "Расстояние" от текущего до pv - сколько надо прибавить к текущему, чтобы получить pv => *pv > *this => dist = pv - dist
    virtual ptrdiff_t distanceTo(const VirtualAddress *pv) const override
    {
        const LinearAddress &other = dynamic_cast<const LinearAddress&>(*pv); // Чтобы самим не кидать исключение bad_cast, используем ссылки
        //MARTY_MEM_ASSERT(m_incSize==other.m_incSize); // Разные размерности недопустимы
        checkCompat(other);
        checkDiff(other.m_address-m_address, "the difference in addresses is not a multiple of the type size");
        return ptrdiff_t(other.m_address - m_address) / ptrdiff_t(m_incSize);
    }

    virtual bool equalTo(const VirtualAddress *pv) const override
    {
        const LinearAddress &other = dynamic_cast<const LinearAddress&>(*pv); // Чтобы самим не кидать исключение bad_cast, используем ссылки
        // MARTY_MEM_ASSERT(m_incSize==other.m_incSize); // Разные размерности недопустимы
        checkCompat(other);
        checkDiff(other.m_address-m_address, "the difference in addresses is not a multiple of the type size");
        return other.m_address == m_address;
    }

    virtual SharedVirtualAddress clone() const override
    {
        auto copyOfThis = std::make_shared<LinearAddress>(*this);
        return std::static_pointer_cast<VirtualAddress>(copyOfThis);
    }


    static bool checkTraits(const LinearAddressTraits &traits)
    {
        if (traits.addressBitSize<traits.extraWrapBitSize)
            return false;
        return true;
    }

    static bool checkIncrement(uint64_t incSize)
    {
        return bits::countOnes(incSize)==1 && incSize<=8; // Не поддерживается гранулярность обращения к памяти больше 8 байт
    }

    LinearAddress() {}

    LinearAddress(uint64_t addr, uint64_t inc=1, const LinearAddressTraits &traits=LinearAddressTraits{})
    : m_address(addr)
    , m_incSize(inc)
    , m_traits (traits)
    , m_addressMask(bits::makeMask(int(traits.addressBitSize)))
    , m_extraWrapMask(bits::makeMask(int(traits.extraWrapBitSize)))
    {
        MARTY_MEM_ASSERT(checkTraits(m_traits));
        MARTY_MEM_ASSERT(checkIncrement(m_incSize));
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
