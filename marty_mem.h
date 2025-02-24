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
#include "exceptions.h"
#include "types.h"
#include "utils.h"

//
#include <array>
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
template<typename IntType>
struct MemoryIteratorBaseImpl
{
    uint64_t      address = 0;

    MemoryIteratorBaseImpl() {}

    explicit MemoryIteratorBaseImpl(uint64_t a) : address(a) {}

    // Остальные ctor/op= компилятор сам сгенерит

    operator uint64_t() const
    {
        return address;
    }

        // m_offset += m_incSize;
        // auto offsSaved = m_offset;
        // m_offset &= m_offsetMask;
        // return offsSaved!=m_offset;

    bool perfromAddImpl(ptrdiff_t d)
    {
        auto addrSaved = address;
        address += std::uint64_t(d*sizeof(IntType));
        return d<0 // На самом деле происходит вычитание?
             ? address>addrSaved // для вычитания делаем как в dec - новый адрес больше старого - переполнение
             : address < addrSaved // для сложения делаем как в inc - новый адрес меньше старого - переполнение
             ;
    }

    void perfromAdd(ptrdiff_t d, bool throwOnWrap)
    {
        if (perfromAddImpl(d) && throwOnWrap)
            throw address_wrap("marty::mem::Memory: address wrap happened");
    }

    void inc(bool throwOnWrap)
    {
        perfromAdd(1, throwOnWrap);
    }

    void dec(bool throwOnWrap)
    {
        perfromAdd(-1, throwOnWrap);
    }

    void add(ptrdiff_t d, bool throwOnWrap)
    {
        perfromAdd(d, throwOnWrap);
    }

    void subtract(ptrdiff_t d, bool throwOnWrap)
    {
        perfromAdd(-d, throwOnWrap);
    }

    static
    void checkDiff(uint64_t diff, const char *msg)
    {
        int64_t diffMod = int64_t(diff)<0 ? -int64_t(diff) : int64_t(diff);
        auto sizeofIntType  = sizeof(IntType);
        auto sizeofIntType1 = sizeofIntType - 1u;
        auto mask = bits::makeMask(int(sizeofIntType1));
        auto diffNmask = diffMod&mask;
        if (diffNmask!=0)
            throw invalid_address_difference(msg);
    }

}; // struct MemoryIteratorBaseImpl



//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename IntType> struct MemoryIterator;
template<typename IntType> struct ConstMemoryIterator;
//----------------------------------------------------------------------------



namespace details
{

static constexpr const uint16_t getAlignedValueValidBitsArray16[16] = { 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u, 0x0020u, 0x0040u, 0x0080u, 0x0100u, 0x0200u, 0x0400u, 0x0800u, 0x1000u, 0x2000u, 0x4000u, 0x8000u };
static constexpr const uint16_t getAlignedValueValidBitsArray8 [ 8] = { 0x0003u, 0x000Cu, 0x0030u, 0x00C0u, 0x0300u, 0x0C00u, 0x3000u, 0xC000u };
static constexpr const uint16_t getAlignedValueValidBitsArray4 [ 4] = { 0x000Fu, 0x00F0u, 0x0F00u, 0xF000u };
static constexpr const uint16_t getAlignedValueValidBitsArray2 [ 2] = { 0x00FFu, 0xFF00u };

inline constexpr uint16_t getAlignedValueValidBits1(uint64_t addr)
{
    auto idx = addr & 0x0Fu;
    return getAlignedValueValidBitsArray16[idx];
}

inline constexpr uint16_t getAlignedValueValidBits2(uint64_t addr)
{
    auto idx = (addr & 0x0Fu)>>1;
    return getAlignedValueValidBitsArray8[idx];
}

inline constexpr uint16_t getAlignedValueValidBits4(uint64_t addr)
{
    auto idx = (addr & 0x0Fu)>>2;
    return getAlignedValueValidBitsArray4[idx];
}

inline constexpr uint16_t getAlignedValueValidBits8(uint64_t addr)
{
    auto idx = (addr & 0x0Fu)>>3;
    return getAlignedValueValidBitsArray2[idx];
}

} // namespace details

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
struct MemoryTraits
{
    Endianness           endianness         = Endianness::littleEndian; // bigEndian
    MemoryOptionFlags    memoryOptionFlags  = MemoryOptionFlags::defaultFf;

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
    uint64_t                                    m_addressValidMax = 0ull;



    static bool checkTraits(const MemoryTraits &traits)
    {
        return traits.endianness==Endianness::littleEndian || traits.endianness==Endianness::bigEndian;; // bigEndian;
    }

    static constexpr
    uint64_t calcMemParaAlignedIndexClearBitsMask(uint64_t size)
    {
        return bits::makeMask(bits::getMsbPower(size));
    }

    static constexpr
    uint16_t getAlignedValueValidBits(uint64_t addr, uint64_t size)
    {
        return (size==1) ? details::getAlignedValueValidBits1(addr)
                         : (size==2) ? details::getAlignedValueValidBits2(addr)
                                     : (size==4) ? details::getAlignedValueValidBits4(addr)
                                                 : details::getAlignedValueValidBits8(addr);
    }

    static
    uint64_t calcParaAddress(uint64_t addr)
    {
        return addr&~0x0Full; // TODO: Проверить
    }

    static constexpr
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

    memory_map_type::iterator getWriteMemIterator(uint64_t addr) const
    {
        auto ncThis = const_cast<Memory*>(this);
        if (m_cachedWriteIter==ncThis->m_memMap.end())
        {
            m_cachedWriteIter = ncThis->m_memMap.find(calcParaAddress(addr));
            return m_cachedWriteIter;
        }
        else if (m_cachedWriteIter->first==calcParaAddress(addr))
        {
            return m_cachedWriteIter;
        }
        else
        {
            m_cachedWriteIter = ncThis->m_memMap.find(calcParaAddress(addr));
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
    MemoryAccessResultCode readAlignedImpl(uint64_t *pResVal, uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead) const
    {
        MARTY_MEM_ASSERT(size==1u || size==2u || size==4u || size==8u);

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
                    *pResVal = getDefaultValue(addr, size, memoryOptionFlags);
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

        uint64_t resVal = 0;

        auto idxBase = calcMemParaAlignedIndex(addr, size);
        uint64_t testAddr = (addr&0x0Full)+idxBase;
        for(std::size_t i=0u; i!=size; ++i)
        {
            uint64_t prevTestAddr = testAddr++;
            if (prevTestAddr>testAddr && (memoryOptionFlags&MemoryOptionFlags::errorOnAddressWrap)!=0)
                return MemoryAccessResultCode::addressWrap;

            resVal <<= 8;
            resVal |= it->second.bytes[idxBase+i];
        }

        if (pResVal)
            *pResVal = resVal;

        return MemoryAccessResultCode::accessGranted;
    }

    // Не кидает исключений, не производит конвертацию в/из big-endian
    MemoryAccessResultCode writeAlignedImpl(uint64_t val, uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::write)
    {
        MARTY_MEM_ASSERT(size==1u || size==2u || size==4u || size==8u);

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
            uint8_t fill = ((m_memoryTraits.memoryOptionFlags&MemoryOptionFlags::defaultFf)!=0) ? uint8_t(0xFFu) : uint8_t(0u);
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
        m_addressValidMax = std::max(m_addressValidMax, addr+size-1u);

        // Ставим биты валидности
        auto alignedValueValidBits = getAlignedValueValidBits(addr, size);
        it->second.validBits |= alignedValueValidBits;

        auto idxBase = calcMemParaAlignedIndex(addr, size);
        uint64_t testAddr = (addr&0x0Full)+idxBase;
        for(std::size_t i=0u; i!=size; ++i, val>>=8)
        {
            uint64_t prevTestAddr = testAddr++;
            if (prevTestAddr>testAddr && (memoryOptionFlags&MemoryOptionFlags::errorOnAddressWrap)!=0)
                return MemoryAccessResultCode::addressWrap;

            it->second.bytes[idxBase+i] = uint8_t(val);
        }

        return MemoryAccessResultCode::accessGranted;
    }



public:

    virtual ~Memory() {}

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
    , m_cachedReadIter(std::exchange(other.m_cachedReadIter, m_memMap.end()))
    , m_cachedWriteIter(std::exchange(other.m_cachedWriteIter, m_memMap.end()))
    , m_addressValidMin(std::exchange(other.m_addressValidMin, 0xFFFFFFFFFFFFFFFFull))
    , m_addressValidMax(std::exchange(other.m_addressValidMax, 0ull))
    {
    }

    Memory& operator=(Memory && other)
    {
        std::exchange(m_memMap, other.m_memMap);
        std::exchange(m_memoryTraits, other.m_memoryTraits);
        std::exchange(m_cachedReadIter, other.m_cachedReadIter);
        std::exchange(m_cachedWriteIter, other.m_cachedWriteIter);
        std::exchange(m_addressValidMin, other.m_addressValidMin);
        std::exchange(m_addressValidMax, other.m_addressValidMax);

        return *this;
    }

    const MemoryTraits& getMemoryTraits() const { return m_memoryTraits; }

    virtual MemoryAccessResultCode checkAccessRights(uint64_t addr, uint64_t size, MemoryAccessRights requestedMode) const
    {
        // В наследнике тут можно проверить права доступа к региону памяти
        MARTY_USED(addr);
        MARTY_USED(size);
        MARTY_USED(requestedMode);

        return MemoryAccessResultCode::accessGranted;
    }

    virtual uint64_t getDefaultValue(uint64_t addr, uint64_t size, MemoryOptionFlags memoryOptionFlags) const
    {
        MARTY_USED(addr);
        // В наследнике, в зависимости от назначения региона памяти, можно возвращать разные значения.
        // Так, при симуляции STM32 чистая флешка возвращает 0xFF, а ОЗУ - нули
        if ((memoryOptionFlags&MemoryOptionFlags::defaultFf)!=0)
            return bits::makeByteSizeMask(int(size));
        return 0;
    }



    template< typename IntType, typename std::enable_if< std::is_integral< IntType >::value, bool>::type = true >
    MemoryAccessResultCode read(IntType *pResVal, uint64_t addr, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        uint64_t val64 = 0;
        if (checkAddressAligned(addr, sizeof(IntType)))
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
            for(auto i=0u; i!=size; ++i)
            {
                val64 <<= 8;
                uint8_t byte = 0;
                auto res = read(&byte, addr, memoryOptionFlags, requestedMode);
                if (res!=MemoryAccessResultCode::accessGranted)
                    return res;
                val64 |= byte;
                uint64_t prevAddr = addr++;
                if (prevAddr>addr && (memoryOptionFlags&MemoryOptionFlags::errorOnAddressWrap)!=0)
                    return MemoryAccessResultCode::addressWrap;
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

    template< typename IntType, typename std::enable_if< std::is_integral< IntType >::value, bool>::type = true >
    MemoryAccessResultCode write(IntType val, uint64_t addr, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::write)
    {
        memoryOptionFlags &= MemoryOptionFlags::writeSimulate; // Чтобы случайно не просочилось

        if (m_memoryTraits.endianness==Endianness::bigEndian)
        {
            val = bits::swapBytes(val);
        }

        uint64_t val64 = uint64_t(val);

        if (checkAddressAligned(addr, sizeof(IntType)))
        {
            return writeAlignedImpl(val64, addr, sizeof(IntType), memoryOptionFlags, requestedMode);
        }

        if ((memoryOptionFlags&MemoryOptionFlags::restrictUnalignedAccess)!=0) // Разрешен только выровненный доступ?
            return MemoryAccessResultCode::unalignedMemoryAccess; // Тогда облом


        // А вот тут надо побайтно писать.

        // Для начала цикл в холостую - если какая ошибка выскочит, то ничего фактически изменено не будет

        uint64_t orgAddr  = addr;
        // uint64_t orgVal64 = val64;
        std::size_t size = sizeof(IntType);
        for(auto i=0u; i!=size; ++i /* , val64>>=8 */ ) // реальное значение можно не вычислять - всё равно записи не производится
        {
            auto res = writeAlignedImpl( 0 /* val64 */ , addr, 1, memoryOptionFlags|MemoryOptionFlags::writeSimulate, requestedMode);
            if (res!=MemoryAccessResultCode::accessGranted)
                return res;
            uint64_t prevAddr = addr++;
            if (prevAddr>addr && (memoryOptionFlags&MemoryOptionFlags::errorOnAddressWrap)!=0)
                return MemoryAccessResultCode::addressWrap;
        }

        addr  = orgAddr ;
        // val64 = orgVal64;
        for(auto i=0u; i!=size; ++i, ++addr, val64>>=8)
        {
            // Результат можно не проверять
            writeAlignedImpl(val64, addr, 1, memoryOptionFlags, requestedMode);
        }

        return MemoryAccessResultCode::accessGranted;

    }

    template< typename IntType, typename std::enable_if< std::is_integral< IntType >::value, bool>::type = true >
    MemoryAccessResultCode read(IntType *pResVal, uint64_t addr, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return read(pResVal, addr, m_memoryTraits.memoryOptionFlags, requestedMode);
    }

    template< typename IntType, typename std::enable_if< std::is_integral< IntType >::value, bool>::type = true >
    MemoryAccessResultCode write(IntType val, uint64_t addr, MemoryAccessRights requestedMode=MemoryAccessRights::write)
    {
        return write(val, addr, m_memoryTraits.memoryOptionFlags, requestedMode);
    }

    MemoryAccessResultCode read(byte_vector_t &v, uint64_t addr, uint64_t nRead, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        v.reserve(std::size_t(nRead));

        for(uint64_t i=0u; i!=nRead; ++i, ++addr)
        {
            byte_t b = 0;
            auto rc = read(&b, addr, memoryOptionFlags, requestedMode);
            if (rc!=MemoryAccessResultCode::accessGranted)
                return rc;
            v.push_back(b);
        }

        return MemoryAccessResultCode::accessGranted;
    }

    MemoryAccessResultCode read(byte_vector_t &v, uint64_t addr, uint64_t nRead, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return read(v, addr, nRead, m_memoryTraits.memoryOptionFlags, requestedMode);
    }

    MemoryAccessResultCode write(const byte_vector_t &v, uint64_t addr, uint64_t nWrite, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        memoryOptionFlags &= MemoryOptionFlags::writeSimulate; // Чтобы случайно не просочилось

        if (nWrite>v.size())
            nWrite = v.size();

        uint64_t addrOrg = addr;
        for(uint64_t i=0u; i!=nWrite; ++i, ++addr)
        {
            auto rc = write(v[i], addr, memoryOptionFlags|MemoryOptionFlags::writeSimulate, requestedMode);
            if (rc!=MemoryAccessResultCode::accessGranted)
                return rc;
        }

        addr = addrOrg;
        for(uint64_t i=0u; i!=nWrite; ++i, ++addr)
        {
            write(v[i], addr, memoryOptionFlags, requestedMode);
        }

        return MemoryAccessResultCode::accessGranted;
    }

    MemoryAccessResultCode write(const byte_vector_t &v, uint64_t addr, uint64_t nWrite, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return write(v, addr, nWrite, m_memoryTraits.memoryOptionFlags, requestedMode);
    }

    MemoryAccessResultCode write(const byte_vector_t &v, uint64_t addr, MemoryOptionFlags memoryOptionFlags, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return write(v, addr, v.size(), memoryOptionFlags, requestedMode);
    }

    MemoryAccessResultCode write(const byte_vector_t &v, uint64_t addr, MemoryAccessRights requestedMode=MemoryAccessRights::executeRead)
    {
        return write(v, addr, v.size(), m_memoryTraits.memoryOptionFlags, requestedMode);
    }


    uint64_t addressMin() const { return m_addressValidMin; }
    uint64_t addressMax() const { return m_addressValidMax; }
    bool     addressMinMaxValid() const { return m_addressValidMin<=m_addressValidMax; }

    bool     empty() const { return m_memMap.empty(); }
    
    uint64_t addressBegin() const { return m_addressValidMin; }
    uint64_t addressEnd()   const { return empty() ? m_addressValidMin : m_addressValidMax+1; }

    template<typename IntType>
    uint64_t addressEndAligned() const
    {
        uint64_t diff = addressEnd() - addressBegin();
        auto mask = bits::makeMask(unsigned(sizeof(IntType)-1u));
        diff &= ~mask;
        return addressBegin()+diff;
    }

    template<typename IntType=byte_t> MemoryIterator<IntType> begin(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss);
    template<typename IntType=byte_t> MemoryIterator<IntType> end(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss);

    template<typename IntType=byte_t> ConstMemoryIterator<IntType> begin(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;
    template<typename IntType=byte_t> ConstMemoryIterator<IntType> end(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;

    template<typename IntType=byte_t> ConstMemoryIterator<IntType> cbegin(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;
    template<typename IntType=byte_t> ConstMemoryIterator<IntType> cend(MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;

    template<typename IntType=byte_t> MemoryIterator<IntType>      iterator(uint64_t addr, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss);
    template<typename IntType=byte_t> ConstMemoryIterator<IntType> iterator(uint64_t addr, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;
    template<typename IntType=byte_t> ConstMemoryIterator<IntType> citerator(uint64_t addr, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) const;


}; // class Memory

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename IntType>
struct MemoryIterator : public MemoryIteratorBaseImpl<IntType>
{

    using BaseImpl = MemoryIteratorBaseImpl<IntType>;

    Memory              *pMemory = 0;
    MemoryOptionFlags    memoryOptionFlags = MemoryOptionFlags::none;


    struct AccessProxy
    {
        Memory             *pMemory = 0;
        uint64_t            address = 0;
        MemoryOptionFlags   memoryOptionFlags = 0;

        AccessProxy() {}

        explicit AccessProxy(Memory *pm, std::uint64_t addr, MemoryOptionFlags mof) : pMemory(pm), address(addr), memoryOptionFlags(mof)
        {
            MARTY_MEM_ASSERT(pMemory);
        }

        AccessProxy& operator=(IntType b)
        {
            MARTY_MEM_ASSERT(pMemory);
            auto rc = pMemory->write(b, address, memoryOptionFlags);
            throwMemoryAccessError(rc);
            return *this;
        }

        operator IntType() const
        {
            
            IntType resVal = 0;
            auto rc = pMemory->read(&resVal, address, memoryOptionFlags);
            throwMemoryAccessError(rc);
            return resVal;
        }
    
    }; // struct AccessProxy


    MemoryIterator() {}

    explicit MemoryIterator(Memory *pm, uint64_t addr, MemoryOptionFlags mof) : BaseImpl(addr), pMemory(pm)
    {
        MARTY_MEM_ASSERT(pMemory);

        mof &= MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss; // Пропускаем извне только эти флаги
        memoryOptionFlags  = pMemory->getMemoryTraits().memoryOptionFlags;
        memoryOptionFlags &= ~(MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss); // В опциях memory сбрасываем эти флаги, не используем дефолтные установки
        memoryOptionFlags |= mof;
    }

    // Остальные ctor/op= компилятор сам сгенерит

    bool getThrowOnWrapOption() const
    {
        return (memoryOptionFlags & MemoryOptionFlags::errorOnAddressWrap)!=0;
    }

    operator std::uint64_t() const               { return BaseImpl::address; }

    operator std::string() const
    {
        return utils::makeHexString<std::string>(BaseImpl::address, std::size_t(8)); // pass num bytes
    }


    MemoryIterator& operator++()     /* pre */   { BaseImpl::inc(getThrowOnWrapOption()); return *this; }
    MemoryIterator& operator--()     /* pre */   { BaseImpl::dec(getThrowOnWrapOption()); return *this; }
    MemoryIterator  operator++(int)  /* post */  { auto cp = *this; BaseImpl::inc(getThrowOnWrapOption()); return cp; }
    MemoryIterator  operator--(int)  /* post */  { auto cp = *this; BaseImpl::dec(getThrowOnWrapOption()); return cp; }
    MemoryIterator& operator+=(ptrdiff_t d)      { BaseImpl::add     (d, getThrowOnWrapOption()); return *this; }
    MemoryIterator& operator-=(ptrdiff_t d)      { BaseImpl::subtract(d, getThrowOnWrapOption()); return *this; }

    AccessProxy operator*()
    {
        return AccessProxy(pMemory, BaseImpl::address, memoryOptionFlags);
    }

}; // struct MemoryIterator


template<typename IntType> MemoryIterator<IntType> operator+(MemoryIterator<IntType> it, ptrdiff_t d) { it += d; return it; }
template<typename IntType> MemoryIterator<IntType> operator+(ptrdiff_t d, MemoryIterator<IntType> it) { it += d; return it; }
template<typename IntType> MemoryIterator<IntType> operator-(MemoryIterator<IntType> it, ptrdiff_t d) { it -= d; return it; }

template<typename IntType> ptrdiff_t operator-(MemoryIterator<IntType> it1, MemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't calculate MemoryIterator difference: difference of iterators is not equal to size of type");
    return ptrdiff_t(delta/sizeof(IntType));
}

template<typename IntType> bool operator==(MemoryIterator<IntType> it1, MemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't compare MemoryIterator's: difference of iterators is not equal to size of type");
    return it1.address==it2.address;
}

template<typename IntType> bool operator!=(MemoryIterator<IntType> it1, MemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't compare MemoryIterator's: difference of iterators is not equal to size of type");
    return it1.address!=it2.address;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename IntType>
struct ConstMemoryIterator : public MemoryIteratorBaseImpl<IntType>
{

    using BaseImpl = MemoryIteratorBaseImpl<IntType>;

    const Memory        *pMemory = 0;
    MemoryOptionFlags    memoryOptionFlags = MemoryOptionFlags::none;


    struct AccessProxy
    {
        const Memory       *pMemory = 0;
        uint64_t            address = 0;
        MemoryOptionFlags   memoryOptionFlags = 0; // bool throwOnHitMiss

        AccessProxy() {}

        explicit AccessProxy(const Memory *pm, std::uint64_t addr, MemoryOptionFlags mof) : pMemory(pm), address(addr), memoryOptionFlags(mof)
        {
            MARTY_MEM_ASSERT(pMemory);
        }

        operator IntType() const
        {
            MARTY_MEM_ASSERT(pMemory);
            IntType resVal = 0;
            auto rc = pMemory->read(&resVal, address, memoryOptionFlags);
            throwMemoryAccessError(rc);
            return resVal;
        }
    
    }; // struct AccessProxy


    ConstMemoryIterator() {}

    explicit ConstMemoryIterator(const Memory *pm, uint64_t addr, MemoryOptionFlags mof) : BaseImpl(addr), pMemory(pm)
    {
        MARTY_MEM_ASSERT(pMemory);
     
        mof &= MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss; // Пропускаем извне только эти флаги
        memoryOptionFlags  = pMemory->getMemoryTraits().memoryOptionFlags;
        memoryOptionFlags &= ~(MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss); // В опциях memory сбрасываем эти флаги, не используем дефолтные установки
        memoryOptionFlags |= mof;
    }

    ConstMemoryIterator(MemoryIterator<IntType> it) : BaseImpl(it.address), pMemory(it.pMemory), memoryOptionFlags(it.memoryOptionFlags) {}
    
    // Остальные ctor/op= компилятор сам сгенерит

    bool getThrowOnWrapOption() const
    {
        return (memoryOptionFlags & MemoryOptionFlags::errorOnAddressWrap)!=0;
    }

    operator std::uint64_t() const               { return BaseImpl::address; }

    operator std::string() const
    {
        return utils::makeHexString<std::string>(BaseImpl::address, std::size_t(8)); // pass num bytes
    }


    ConstMemoryIterator& operator++()     /* pre */   { BaseImpl::inc(getThrowOnWrapOption()); return *this; }
    ConstMemoryIterator& operator--()     /* pre */   { BaseImpl::dec(getThrowOnWrapOption()); return *this; }
    ConstMemoryIterator  operator++(int)  /* post */  { auto cp = *this; BaseImpl::inc(getThrowOnWrapOption()); return cp; }
    ConstMemoryIterator  operator--(int)  /* post */  { auto cp = *this; BaseImpl::dec(getThrowOnWrapOption()); return cp; }
    ConstMemoryIterator& operator+=(ptrdiff_t d)      { BaseImpl::add     (d, getThrowOnWrapOption()); return *this; }
    ConstMemoryIterator& operator-=(ptrdiff_t d)      { BaseImpl::subtract(d, getThrowOnWrapOption()); return *this; }

    AccessProxy operator*()
    {
        return AccessProxy(pMemory, BaseImpl::address, memoryOptionFlags);
    }

}; // struct ConstMemoryIterator


template<typename IntType> ConstMemoryIterator<IntType> operator+(ConstMemoryIterator<IntType> it, ptrdiff_t d) { it += d; return it; }
template<typename IntType> ConstMemoryIterator<IntType> operator+(ptrdiff_t d, ConstMemoryIterator<IntType> it) { it += d; return it; }
template<typename IntType> ConstMemoryIterator<IntType> operator-(ConstMemoryIterator<IntType> it, ptrdiff_t d) { it -= d; return it; }

template<typename IntType> ptrdiff_t operator-(ConstMemoryIterator<IntType> it1, ConstMemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't calculate ConstMemoryIterator difference: difference of iterators is not equal to size of type");
    return ptrdiff_t(delta/sizeof(IntType));
}

template<typename IntType> bool operator==(ConstMemoryIterator<IntType> it1, ConstMemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't compare ConstMemoryIterator's: difference of iterators is not equal to size of type");
    return it1.address==it2.address;
}

template<typename IntType> bool operator!=(ConstMemoryIterator<IntType> it1, ConstMemoryIterator<IntType> it2)
{
    uint64_t delta = it1.address - it2.address;
    it1.checkDiff(delta, "can't compare ConstMemoryIterator's: difference of iterators is not equal to size of type");
    return it1.address!=it2.address;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename IntType> MemoryIterator<IntType>      Memory::begin(MemoryOptionFlags memoryOptionFlags)        { return MemoryIterator<IntType>(this, addressBegin(), memoryOptionFlags); }
template<typename IntType> MemoryIterator<IntType>      Memory::end(MemoryOptionFlags memoryOptionFlags)          { return MemoryIterator<IntType>(this, addressEndAligned<IntType>(), memoryOptionFlags); }
 
template<typename IntType> ConstMemoryIterator<IntType> Memory::begin(MemoryOptionFlags memoryOptionFlags)  const { return ConstMemoryIterator<IntType>(this, addressBegin(), memoryOptionFlags); }
template<typename IntType> ConstMemoryIterator<IntType> Memory::end(MemoryOptionFlags memoryOptionFlags)    const { return ConstMemoryIterator<IntType>(this, addressEndAligned<IntType>(), memoryOptionFlags); }
 
template<typename IntType> ConstMemoryIterator<IntType> Memory::cbegin(MemoryOptionFlags memoryOptionFlags) const { return ConstMemoryIterator<IntType>(this, addressBegin(), memoryOptionFlags); }
template<typename IntType> ConstMemoryIterator<IntType> Memory::cend(MemoryOptionFlags memoryOptionFlags)   const { return ConstMemoryIterator<IntType>(this, addressEndAligned<IntType>(), memoryOptionFlags); }

template<typename IntType> MemoryIterator<IntType>      Memory::iterator(uint64_t addr, MemoryOptionFlags memoryOptionFlags)        { return MemoryIterator<IntType>(this, addr, memoryOptionFlags); }
template<typename IntType> ConstMemoryIterator<IntType> Memory::iterator(uint64_t addr, MemoryOptionFlags memoryOptionFlags)  const { return ConstMemoryIterator<IntType>(this, addr, memoryOptionFlags); }
template<typename IntType> ConstMemoryIterator<IntType> Memory::citerator(uint64_t addr, MemoryOptionFlags memoryOptionFlags) const { return ConstMemoryIterator<IntType>(this, addr, memoryOptionFlags); }


// MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

