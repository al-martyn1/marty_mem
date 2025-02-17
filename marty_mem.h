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
    byte_t     para[16]  = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

}; // struct MemPara

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class Memory
{

    std::unordered_map<uint64_t, MemPara>   m_memMap;
    MemoryTraits                            m_memoryTraits;

    uint64_t                                m_addressValidMin = 0xFFFFFFFFFFFFFFFFull;
    uint64_t                                m_addressValidMax = 0;

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
        idx &= ~calcMemParaAlignedIndexClearBitsMask(size); // TODO: Проверить
        return std::size_t(idx);
    }


    // Не кидает исключений, не производит конвертацию в/из big-endian
    MemoryAccessResultCode readAlignedImpl(uint64_t *pResVal, uint64_t addr, uint64_t size, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        auto res = checkAccessRights(addr, sizeof(*pVal), requestedMode);
        if (res!=MemoryAccessResultCode::accessGranted)
            return res;

        if ((addr&calcMemParaAlignedIndexClearBitsMask(size))!=0)
            return MemoryAccessResultCode::unalignedMemoryAccess; // TODO: Проверить


        uint64_t paraAddr = calcParaAddress(addr);

        std::unordered_map<uint64_t, MemPara>::const_iterator it = m_memMap.find(paraAddr);
        if (it==m_memMap.end())
            return MemoryAccessResultCode::unassignedMemoryAccess;



        //uint16_t getAlignedValueValidBits(uint64_t addr, uint64_t size)

        
    }


    // MemoryOptionFlags    memoryOptionFlags  = MemoryOptionFlags::preciseHitMiss | MemoryOptionFlags::throwOnHitMiss | MemoryOptionFlags::defaultFF;

    // uint64_t calcParaAddress(uint64_t addr)
    // uint16_t getAlignedValueValidBits<uint32_t>(uint64_t addr)



public:

    Memory() {}

    Memory(const MemoryTraits &memTraits) : m_memMap(), m_memoryTraits(memTraits)
    {
        // check traits here
        MARTY_MEM_ASSERT(checkTraits(m_memoryTraits));

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










}; // class Memory

// unaligned access невыровненный доступ
// aligned access   согласованный доступ


//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

