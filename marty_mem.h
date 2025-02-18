/*! \file
    \brief Симуляция памяти
 */

#pragma once

//----------------------------------------------------------------------------
/*
    Память представляем в виде unordered_map

*/

//----------------------------------------------------------------------------
#include "assert.h"
#include "bits.h"
#include "enums.h"
#include "types.h"
#include "utils.h"

//
#include <algorithm>
#include <unordered_map>
#include <utility>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/marty_mem.h"
// marty::mem::
namespace marty{
namespace mem{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
struct MemoryTraits
{
    Endianness           endianness         = Endianness::littleEndian; // bigEndian
    MemoryOptionFlags    memoryOptionFlags  = MemoryOptionFlags::preciseHitMiss | MemoryOptionFlags::throwOnHitMiss | MemoryOptionFlags::defaultFF;

}; // struct MemoryTraits


//----------------------------------------------------------------------------
struct MemPara
{
    uint16_t   validBits = 0; // единичный бит говорит, что память была ранее присвоена, 0 - память неинициализирована. Младшие биты соответствуют младшим адресам
    byte_t     bytes[16]  = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

}; // struct MemPara

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class Memory
{
    using memory_map_type = std::unordered_map<uint64_t, MemPara>;

    memory_map_type                             m_memMap;
    MemoryTraits                                m_memoryTraits;

    // Кешируем итераторы, чтобы при последовательном доступе поиск не производился
    // Да, он вроде бы линейный, но тем не менее
    mutable memory_map_type::const_iterator     m_cachedReadIter ;
    mutable memory_map_type::iterator           m_cachedWriteIter;

    uint64_t                                    m_addressValidMin = 0xFFFFFFFFFFFFFFFFull;
    uint64_t                                    m_addressValidMax = 0;



    static bool checkTraits(const MemoryTraits &traits)
    {
        return endianness==Endianness::littleEndian || endianness==Endianness::bigEndian;; // bigEndian;
    }

    static
    uint64_t calcMemParaAlignedIndexClearBitsMask(uint64_t size)
    {
        MARTY_MEM_ASSERT(size==1u || size==2u || size==4u || size==8u);
        // 1 - 0
        // 2 - 1
        // 4 - 2
        // 8 - 3
        return bits::makeMask(bits::getMsbPower(size));
    }


    template<typename IntType>
    static
    uint16_t getAlignedValueValidBits(uint64_t addr)
    {
        constexpr const static uint16_t validBits[16] = { 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u, 0x0020u, 0x0040u, 0x0080u, 0x0100u, 0x0200u, 0x0400u, 0x0800u, 0x1000u, 0x2000u, 0x4000u, 0x8000u };
        auto idx = addr & 0x0Fu;
        return validBits[idx];
    }

    template<>
    static
    uint16_t getAlignedValueValidBits<uint16_t>(uint64_t addr)
    {
        constexpr const static uint16_t validBits[8] = { 0x0003u, 0x000Cu, 0x0030u, 0x00C0u, 0x0300u, 0x0C00u, 0x3000u, 0xC000u };
        auto idx = (addr & 0x0Fu)>>1;
        return validBits[idx];
    }

    template<>
    static
    uint16_t getAlignedValueValidBits<uint32_t>(uint64_t addr)
    {
        constexpr const static uint16_t validBits[4] = { 0x000Fu, 0x00F0u, 0x0F00u, 0xF000u };
        auto idx = (addr & 0x0Fu)>>2;
        return validBits[idx];
    }

    template<>
    static
    uint16_t getAlignedValueValidBits<uint64_t>(uint64_t addr)
    {
        constexpr const static uint16_t validBits[2] = { 0x00FFu, 0xFF00u };
        auto idx = (addr & 0x0Fu)>>3;
        return validBits[idx];
    }

    static
    uint16_t getAlignedValueValidBits(uint64_t addr, uint64_t size)
    {
        MARTY_MEM_ASSERT(size==1u || size==2u || size==4u || size==8u);

        return (size==1) ? getAlignedValueValidBits<uint8_t>(addr)
                         : (size==2) ? getAlignedValueValidBits<uint16_t>(addr)
                                     : (size==4) ? getAlignedValueValidBits<uint32_t>(addr)
                                                 : getAlignedValueValidBits<uint64_t>(addr);
    }

    static
    uint64_t calcParaAddress(uint64_t addr)
    {
        return addr&~0x0Full; // TODO: Проверить
    }

    static
    std::size_t calcMemParaAlignedIndex(uint64_t addr, uint64_t size)
    {
        uint64_t idx = addr&0x0Full;
        auto alignmentBits = calcMemParaAlignedIndexClearBitsMask(size);
        idx &= ~alignmentBits; // TODO: Проверить
        return std::size_t(idx);
    }

    memory_map_type::const_iterator getReadMemIterator(uint64_t addr) const
    {
        if (m_cachedReadIter==m_memMap.end())
        {
            m_cachedReadIter = m_memMap.find(calcParaAddress(addr));
            return m_cachedReadIter;
        }
        else if (m_cachedReadIter->first==calcParaAddress(addr))
        {
            return m_cachedReadIter;
        }
        else
        {
            m_cachedReadIter = m_memMap.find(calcParaAddress(addr));
            return m_cachedReadIter;
        }
    }

    memory_map_type::iterator getWriteMemIterator(uint64_t addr)
    {
        if (m_cachedWriteIter==m_memMap.end())
        {
            m_cachedWriteIter = m_memMap.find(calcParaAddress(addr));
            return m_cachedWriteIter;
        }
        else if (m_cachedWriteIter->first==calcParaAddress(addr))
        {
            return m_cachedWriteIter;
        }
        else
        {
            m_cachedWriteIter = m_memMap.find(calcParaAddress(addr));
            return m_cachedWriteIter;
        }
    }

    static
    bool checkAddressAligned(uint64_t addr, uint64_t size)
    {
        auto alignmentBits = calcMemParaAlignedIndexClearBitsMask(size);
        return ((addr&alignmentBits)==0);
    }


    // Не кидает исключений, не производит конвертацию в/из big-endian
    MemoryAccessResultCode readAlignedImpl(uint64_t *pResVal, uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        auto res = checkAccessRights(addr, sizeof(*pResVal), requestedMode);
        if (res!=MemoryAccessResultCode::accessGranted)
            return res;

        if (!checkAddressAligned(addr, size))
            return MemoryAccessResultCode::unalignedMemoryAccess; // TODO: Проверить

        auto it = getReadMemIterator(addr);
        if (it==m_memMap.end())
        {
            if ((memoryOptionFlags&MemoryOptionFlags::errorOnHitMiss)!=0) // Иначе - допустимо, и вернём на месте пустых байт 0 или 0xFF
            {
                return MemoryAccessResultCode::unassignedMemoryAccess;
            }
            else
            {
                if (pResVal)
                {
                    *pResVal = 0;
                    if ((memoryOptionFlags&MemoryOptionFlags::defaultFf)!=0) // Врзвращаем число, байты которого заполнены 0xFF
                        *pResVal = makeByteSizeMask(size);
                }

                return MemoryAccessResultCode::accessGranted;
            }
        }

        // Забиваем на preciseHitMiss
        auto alignedValueValidBits = getAlignedValueValidBits(addr, size);
        if ((it->second.validBits&alignedValueValidBits)!=alignedValueValidBits) // всё биты годные?
        {
            if ((memoryOptionFlags&MemoryOptionFlags::errorOnHitMiss)!=0) // Иначе - допустимо, и вернём на месте пустых байт 0 или 0xFF
            {
                return MemoryAccessResultCode::unassignedMemoryAccess; 
            }
        }

        if (!pResVal)
            return MemoryAccessResultCode::accessGranted;

        uint64_t resVal = 0;

        auto idxBase = calcMemParaAlignedIndex(addr, size);
        for(std::size_t i=0u; i!=size; ++i, resVal<<=8)
        {
            resVal |= it->second.bytes[idxBase+i];
        }

        *pResVal = resVal;

        return MemoryAccessResultCode::accessGranted;
    }

    // Не кидает исключений, не производит конвертацию в/из big-endian
    MemoryAccessResultCode writeAlignedImpl(uint64_t val, uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::write)
    {
        auto res = checkAccessRights(addr, sizeof(val), requestedMode);
        if (res!=MemoryAccessResultCode::accessGranted)
            return res;

        if (!checkAddressAligned(addr, size))
            return MemoryAccessResultCode::unalignedMemoryAccess; // TODO: Проверить

        auto it = getWriteMemIterator(addr); // Всегда дёргаем итератор

        if ((memoryOptionFlags&MemoryOptionFlags::writeSimulate)!=0)
        {
            return MemoryAccessResultCode::accessGranted; // Фактическую запись не производим
        }

        if (it==m_memMap.end())
        {
            MemPara mp;
            mp.validBits = 0;
            uint8_t fill = ((m_memoryTraits.memoryOptionFlags&MemoryOptionFlags::defaultFF)!=0) ? uint8_t(0xFFu) : uint8_t(0u);
            // m_memoryTraits.memoryOptionFlags  = MemoryOptionFlags::preciseHitMiss | MemoryOptionFlags::throwOnHitMiss | MemoryOptionFlags::defaultFF;
            for(auto i=0u; i!=16u; ++i)
            {
                mp.bytes[i] = fill;
            }

            auto p = m_memMap.insert(std::make_pair(calcParaAddress(addr), mp));
            it = m_cachedWriteIter = p.first;
        }

        // Обновляем диапазон адресов
        m_addressValidMin = std::min(m_addressValidMin, addr);
        m_addressValidMax = std::max(m_addressValidMin, addr+size-1u);

        // Ставим биты валидности
        auto alignedValueValidBits = getAlignedValueValidBits(addr, size);
        it->second.validBits |= alignedValueValidBits;

        auto idxBase = calcMemParaAlignedIndex(addr, size);
        for(std::size_t i=0u; i!=size; ++i, val>>=8)
        {
            it->second.bytes[idxBase+i] = uint8_t(val);
        }

        return MemoryAccessResultCode::accessGranted;
    }



public:

    Memory() : m_memMap(), m_cachedReadIter(m_memMap.end()), m_cachedWriteIter(m_memMap.end()) {}

    Memory(const MemoryTraits &memTraits)
    : m_memMap(), m_memoryTraits(memTraits)
    , m_cachedReadIter(m_memMap.end()), m_cachedWriteIter(m_memMap.end())
    {
        // check traits here
        MARTY_MEM_ASSERT(checkTraits(m_memoryTraits));
    }

    Memory(const Memory &other)
    : m_memMap(other.m_memMap), m_memoryTraits(other.m_memoryTraits)
    , m_cachedReadIter(m_memMap.end()), m_cachedWriteIter(m_memMap.end())
    , m_addressValidMin(other.m_addressValidMin)
    , m_addressValidMax(other.m_addressValidMax)
    {}

    Memory& operator=(const Memory &other)
    {
        if (&other==this)
            return *this;

        m_memMap = other.m_memMap;
        m_memoryTraits = other.m_memoryTraits;
        m_cachedReadIter  = m_memMap.end();
        m_cachedWriteIter = m_memMap.end();
        m_addressValidMin = other.m_addressValidMin;
        m_addressValidMax = other.m_addressValidMax;

        return *this;
    }

    Memory(Memory && other)
    : m_memMap(std::exchange(other.m_memMap, memory_map_type()))
    , m_memoryTraits(std::exchange(other.m_memoryTraits, MemoryTraits()))
    , m_cachedReadIter(std::exchange(other.m_memoryTraits, m_memMap.end()))
    , m_cachedWriteIter(std::exchange(other.m_memoryTraits, m_memMap.end()))
    , m_addressValidMin(std::exchange(other.m_addressValidMin, 0xFFFFFFFFFFFFFFFFull))
    , m_addressValidMax(std::exchange(other.m_addressValidMax, 0))
    {
    }

    Memory& operator=(Memory && other)
    {
        std::exchange(m_memMap, other.m_memMap);
        std::exchange(m_memoryTraits, other.m_memoryTraits);
        std::exchange(m_cachedReadIter, other.m_memoryTraits);
        std::exchange(m_cachedWriteIter, other.m_memoryTraits);
        std::exchange(m_addressValidMin, other.m_addressValidMin);
        std::exchange(m_addressValidMax, other.m_addressValidMax);

        return *this;
    }


    virtual MemoryAccessResultCode checkAccessRights(uint64_t addr, uint64_t size, MemoryAccessRights requestedMode) const
    {
        // В наследнике тут можно проверить права доступа к региону памяти
        MARTY_USED(addr);
        MARTY_USED(size);
        MARTY_USED(requestedMode);

        return MemoryAccessResultCode::accessGranted;
    }

    virtual uint64_t getDefaultValue(uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags)
    {
        MARTY_USED(addr);
        // В наследнике, в зависимости от назначения региона памяти, можно возвращать разные значения.
        // Так, при симуляции STM32 чистая флешка возвращает 0xFF, а ОЗУ - нули
        if ((memoryOptionFlags&MemoryOptionFlags::defaultFF)!=0)
            return bits::makeByteSizeMask(std::size_t(size));
        return 0;
    }



    uint64_t addressMin() const { return m_addressValidMin; }
    uint64_t addressMax() const { return m_addressValidMax; }
    bool     addressMinMaxValid() const { return m_addressValidMin<=m_addressValidMax; }

    bool     empty() const { !addressMinMaxValid() || m_memMap.empty(); }
    
    uint64_t addressBegin() const { return m_addressValidMin; }
    uint64_t addressEnd()   const { return m_addressValidMax+1; }



    template< typename IntType, typename std::enable_if< std::is_integral< EnumType >::value, bool>::type = true >
    MemoryAccessResultCode read(IntType *pResVal, uint64_t addr, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        uint64_t val64 = 0;
        if (checkAddressAligned(addr, size))
        {
            auto res = readAlignedImpl(&val64, addr, sizeof(IntType), memoryOptionFlags, requestedMode);
            if (res!=MemoryAccessResultCode::accessGranted)
                return res;

        }
        else // Собираем побайтно
        {
            if ((memoryOptionFlags&MemoryOptionFlags::restrictUnalignedAccess)!=0) // Разрешен только выровненный доступ?
                return MemoryAccessResultCode::unalignedMemoryAccess; // Тогда облом

            std::size_t size = sizeof(IntType);
            for(auto i=0u; i!=size; ++i, ++addr, val64<<=8;)
            {
                uint8_t byte = 0;
                auto res = read(&byte, addr, memoryOptionFlags, requestedMode);
                if (res!=MemoryAccessResultCode::accessGranted)
                    return res;
                val64 |= byte;
            }

        }

        if (pResVal)
        {
            *pResVal = IntType(val64);
            if (m_memoryTraits.endianness==Endianness::bigEndian)
            {
                *pResVal = bits::swapBytes(*pResVal);
            }
        }

        return MemoryAccessResultCode::accessGranted;
    }

    template< typename IntType, typename std::enable_if< std::is_integral< EnumType >::value, bool>::type = true >
    MemoryAccessResultCode write(IntType val, uint64_t addr, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        memoryOptionFlags &= MemoryOptionFlags::writeSimulate; // Чтобы случайно не просочилось

        if (m_memoryTraits.endianness==Endianness::bigEndian)
        {
            val = bits::swapBytes(val);
        }

        uint64_t val64 = uint64_t(val);

        if (checkAddressAligned(addr, size))
        {
            return writeAlignedImpl(val64, addr, sizeof(IntType), memoryOptionFlags, requestedMode);
        }

        if ((memoryOptionFlags&MemoryOptionFlags::restrictUnalignedAccess)!=0) // Разрешен только выровненный доступ?
            return MemoryAccessResultCode::unalignedMemoryAccess; // Тогда облом


        // А вот тут надо побайтно писать.

        // Для начала цикл в холостую - если какая ошибка выскочит, то ничего фактически изменено не будет

        uint64_t orgAddr  = addr;
        uint64_t orgVal64 = val64;
        std::size_t size = sizeof(IntType);
        for(auto i=0u; i!=size; ++i, ++addr, val64>>=8;)
        {
            auto res = writeAlignedImpl(val64, addr, 1, memoryOptionFlags|MemoryOptionFlags::writeSimulate, requestedMode);
            if (res!=MemoryAccessResultCode::accessGranted)
                return res;
        }

        addr  = orgAddr ;
        val64 = orgVal64;
        for(auto i=0u; i!=size; ++i, ++addr, val64>>=8;)
        {
            // Результат можно не проверять
            writeAlignedImpl(val64, addr, 1, memoryOptionFlags, requestedMode);
        }

        return MemoryAccessResultCode::accessGranted;

    }

    template< typename IntType, typename std::enable_if< std::is_integral< EnumType >::value, bool>::type = true >
    MemoryAccessResultCode read(IntType *pResVal, uint64_t addr, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return read(pResVal, addr, m_memoryTraits.memoryOptionFlags, requestedMode);
    }

    template< typename IntType, typename std::enable_if< std::is_integral< EnumType >::value, bool>::type = true >
    MemoryAccessResultCode write(IntType val, uint64_t addr, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return write(val, addr, m_memoryTraits.memoryOptionFlags, requestedMode);
    }



}; // class Memory

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

