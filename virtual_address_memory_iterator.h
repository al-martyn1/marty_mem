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
#include "exceptions.h"
#include "fixed_size_types.h"
#include "virtual_address.h"
#include "linear_address.h"
#include "segmented_address.h"
#include "marty_mem.h"


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
template<typename IntType>
struct VirtualAddressMemoryIterator
{

    Memory                 *pMemory = 0;
    MemoryOptionFlags       memoryOptionFlags = MemoryOptionFlags::none;
    SharedVirtualAddress    virtualAddress;
    bool                    lastModificationWrapSign = false; // Признак переполнения при последней операции изменения итератора


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


    VirtualAddressMemoryIterator() {}

    explicit VirtualAddressMemoryIterator(Memory *pm, SharedVirtualAddress va, MemoryOptionFlags mof=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) : pMemory(pm), virtualAddress(va)
    {
        // MARTY_MEM_ASSERT(pMemory); // Можно создавать итераторы с нулевым указателем на память, но нельзя по ним обращаться к памяти

        mof &= MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss | MemoryOptionFlags::errorOnWrapedAddressAccess; // Пропускаем извне только эти флаги
        if (pMemory)
            memoryOptionFlags  = pMemory->getMemoryTraits().memoryOptionFlags;
        // В опциях memory сбрасываем эти флаги, не используем дефолтные установки
        memoryOptionFlags &= ~(MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss | MemoryOptionFlags::errorOnWrapedAddressAccess); 
        memoryOptionFlags |= mof;

        virtualAddress->setIncrement(sizeof(IntType));
    }

    AddressInfo getAddressInfo() const
    {
        return virtualAddress->getAddressInfo();
    }

    VirtualAddressMemoryIterator deepCopy() const
    {
        auto cp = *this;
        cp.virtualAddress = virtualAddress->clone();
        return cp;
    }

    template<typename OtherType>
    explicit VirtualAddressMemoryIterator(const VirtualAddressMemoryIterator<OtherType> &other ) : pMemory(other.pMemory), memoryOptionFlags(other.memoryOptionFlags)
    {
        virtualAddress = other.virtualAddress.deepCopy();
        virtualAddress->setIncrement(sizeof(IntType));
    }

    // Остальные ctor/op= компилятор сам сгенерит

    operator std::uint64_t() const    { return virtualAddress->getLinearAddress(); }
    operator std::string  () const    { return virtualAddress->toString(); }


    bool getThrowOnWrapOption() const { return (memoryOptionFlags & MemoryOptionFlags::errorOnAddressWrap)!=0; }
    bool getThrowOnWrapAccessOption() const { return (memoryOptionFlags & MemoryOptionFlags::errorOnWrapedAddressAccess)!=0; }
    void throwAddressWrap    () const { throwMemoryAccessError(MemoryAccessResultCode::addressWrap); }

    void inc     ()             { lastModificationWrapSign=virtualAddress->inc     ( ); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void dec     ()             { lastModificationWrapSign=virtualAddress->dec     ( ); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void add     (ptrdiff_t d)  { lastModificationWrapSign=virtualAddress->add     (d); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void subtract(ptrdiff_t d)  { lastModificationWrapSign=virtualAddress->subtract(d); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }

    VirtualAddressMemoryIterator& operator++()    /* pre  */  { inc(); return *this; }
    VirtualAddressMemoryIterator  operator++(int) /* post */  { auto cp = deepCopy(); inc(); return cp; }
    VirtualAddressMemoryIterator& operator--()    /* pre  */  { dec(); return *this; }
    VirtualAddressMemoryIterator  operator--(int) /* post */  { auto cp = deepCopy(); dec(); return cp; }
    VirtualAddressMemoryIterator& operator+=(ptrdiff_t d)     { add(d); return *this; }
    VirtualAddressMemoryIterator& operator-=(ptrdiff_t d)     { subtract(d); return *this; }

    AccessProxy operator*()
    {
        if (lastModificationWrapSign && getThrowOnWrapAccessOption())
            throwAddressWrap();
        return AccessProxy(pMemory, virtualAddress->getLinearAddress(), memoryOptionFlags);
    }

}; // struct VirtualAddressMemoryIterator


template<typename IntType> VirtualAddressMemoryIterator<IntType> operator+(const VirtualAddressMemoryIterator<IntType> &it, ptrdiff_t d) { auto cp = it.deepCopy(); cp += d; return cp; }
template<typename IntType> VirtualAddressMemoryIterator<IntType> operator+(ptrdiff_t d, const VirtualAddressMemoryIterator<IntType> &it) { auto cp = it.deepCopy(); cp += d; return cp; }
template<typename IntType> VirtualAddressMemoryIterator<IntType> operator-(const VirtualAddressMemoryIterator<IntType> &it, ptrdiff_t d) { auto cp = it.deepCopy(); cp -= d; return cp; }

template<typename IntType> ptrdiff_t operator-(const VirtualAddressMemoryIterator<IntType> &it1, const VirtualAddressMemoryIterator<IntType> &it2)
{
    return it2.virtualAddress->distanceTo(it1.virtualAddress.get());
}

template<typename IntType> bool operator==(const VirtualAddressMemoryIterator<IntType> &it1, const VirtualAddressMemoryIterator<IntType> &it2)
{
    return it1.virtualAddress->equalTo(it2.virtualAddress.get());
}

template<typename IntType> bool operator!=(const VirtualAddressMemoryIterator<IntType> &it1, const VirtualAddressMemoryIterator<IntType> &it2)
{
    return !it1.virtualAddress->equalTo(it2.virtualAddress.get());
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template<typename IntType>
struct ConstVirtualAddressMemoryIterator
{

    const Memory           *pMemory = 0;
    MemoryOptionFlags       memoryOptionFlags = MemoryOptionFlags::none;
    SharedVirtualAddress    virtualAddress;
    bool                    lastModificationWrapSign = false; // Признак переполнения при последней операции изменения итератора


    struct AccessProxy
    {
        const Memory       *pMemory = 0;
        uint64_t            address = 0;
        MemoryOptionFlags   memoryOptionFlags = 0;

        AccessProxy() {}

        explicit AccessProxy(const Memory *pm, std::uint64_t addr, MemoryOptionFlags mof) : pMemory(pm), address(addr), memoryOptionFlags(mof)
        {
            MARTY_MEM_ASSERT(pMemory);
        }

        operator IntType() const
        {
            IntType resVal = 0;
            auto rc = pMemory->read(&resVal, address, memoryOptionFlags);
            throwMemoryAccessError(rc);
            return resVal;
        }
    
    }; // struct AccessProxy


    ConstVirtualAddressMemoryIterator() {}

    explicit ConstVirtualAddressMemoryIterator(const Memory *pm, SharedVirtualAddress va, MemoryOptionFlags mof=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss) : pMemory(pm), virtualAddress(va)
    {
        //MARTY_MEM_ASSERT(pMemory);

        mof &= MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss | MemoryOptionFlags::errorOnWrapedAddressAccess; // Пропускаем извне только эти флаги
        if (pMemory)
            memoryOptionFlags  = pMemory->getMemoryTraits().memoryOptionFlags;
        // В опциях memory сбрасываем эти флаги, не используем дефолтные установки
        memoryOptionFlags &= ~(MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss | MemoryOptionFlags::errorOnWrapedAddressAccess); 
        memoryOptionFlags |= mof;

        virtualAddress->setIncrement(sizeof(IntType));

        if (!virtualAddress->checkAddressInValidSizeRange() && (memoryOptionFlags&MemoryOptionFlags::errorOnAddressWrap)!=0)
            throwMemoryAccessError(MemoryAccessResultCode::addressWrap);
    }

    AddressInfo getAddressInfo() const
    {
        return virtualAddress->getAddressInfo();
    }

    ConstVirtualAddressMemoryIterator deepCopy() const
    {
        auto cp = *this;
        cp.virtualAddress = virtualAddress->clone();
        return cp;
    }

    template<typename OtherType>
    explicit ConstVirtualAddressMemoryIterator(const ConstVirtualAddressMemoryIterator<OtherType> &other) : pMemory(other.pMemory), memoryOptionFlags(other.memoryOptionFlags)
    {
        virtualAddress = other.virtualAddress.deepCopy();
        virtualAddress->setIncrement(sizeof(IntType));
    }

    template<typename OtherType>
    explicit ConstVirtualAddressMemoryIterator(const VirtualAddressMemoryIterator<OtherType> &other) : pMemory(other.pMemory), memoryOptionFlags(other.memoryOptionFlags)
    {
        virtualAddress = other.virtualAddress.deepCopy();
        virtualAddress->setIncrement(sizeof(IntType));
    }

    ConstVirtualAddressMemoryIterator(const ConstVirtualAddressMemoryIterator &other) : pMemory(other.pMemory), memoryOptionFlags(other.memoryOptionFlags), virtualAddress(other.virtualAddress)
    {
    }

    // Остальные ctor/op= компилятор сам сгенерит

    operator std::uint64_t() const    { return virtualAddress->getLinearAddress(); }
    operator std::string  () const    { return virtualAddress->toString(); }

    bool getThrowOnWrapOption() const { return (memoryOptionFlags & MemoryOptionFlags::errorOnAddressWrap)!=0; }
    bool getThrowOnWrapAccessOption() const { return (memoryOptionFlags & MemoryOptionFlags::errorOnWrapedAddressAccess)!=0; }
    void throwAddressWrap    () const { throwMemoryAccessError(MemoryAccessResultCode::addressWrap); }

    void inc     ()             { lastModificationWrapSign=virtualAddress->inc     ( ); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void dec     ()             { lastModificationWrapSign=virtualAddress->dec     ( ); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void add     (ptrdiff_t d)  { lastModificationWrapSign=virtualAddress->add     (d); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }
    void subtract(ptrdiff_t d)  { lastModificationWrapSign=virtualAddress->subtract(d); if (lastModificationWrapSign && getThrowOnWrapOption()) throwAddressWrap(); }

    ConstVirtualAddressMemoryIterator& operator++()    /* pre  */  { inc(); return *this; }
    ConstVirtualAddressMemoryIterator  operator++(int) /* post */  { auto cp = deepCopy(); inc(); return cp; }
    ConstVirtualAddressMemoryIterator& operator--()    /* pre  */  { dec(); return *this; }
    ConstVirtualAddressMemoryIterator  operator--(int) /* post */  { auto cp = deepCopy(); dec(); return cp; }
    ConstVirtualAddressMemoryIterator& operator+=(ptrdiff_t d)     { add(d); return *this; }
    ConstVirtualAddressMemoryIterator& operator-=(ptrdiff_t d)     { subtract(d); return *this; }

    AccessProxy operator*()
    {
        if (lastModificationWrapSign && getThrowOnWrapAccessOption())
            throwAddressWrap();
        return AccessProxy(pMemory, virtualAddress->getLinearAddress(), memoryOptionFlags);
    }

}; // struct ConstVirtualAddressMemoryIterator


template<typename IntType> ConstVirtualAddressMemoryIterator<IntType> operator+(const ConstVirtualAddressMemoryIterator<IntType> &it, ptrdiff_t d) { auto cp = it.deepCopy(); cp += d; return cp; }
template<typename IntType> ConstVirtualAddressMemoryIterator<IntType> operator+(ptrdiff_t d, const ConstVirtualAddressMemoryIterator<IntType> &it) { auto cp = it.deepCopy(); cp += d; return cp; }
template<typename IntType> ConstVirtualAddressMemoryIterator<IntType> operator-(const ConstVirtualAddressMemoryIterator<IntType> &it, ptrdiff_t d) { auto cp = it.deepCopy(); cp -= d; return cp; }

template<typename IntType> ptrdiff_t operator-(const ConstVirtualAddressMemoryIterator<IntType> &it1, const ConstVirtualAddressMemoryIterator<IntType> &it2)
{
    return it2.virtualAddress->distanceTo(it1.virtualAddress.get());
}

template<typename IntType> bool operator==(const ConstVirtualAddressMemoryIterator<IntType> &it1, const ConstVirtualAddressMemoryIterator<IntType> &it2)
{
    return it1.virtualAddress->equalTo(it2.virtualAddress.get());
}

template<typename IntType> bool operator!=(const ConstVirtualAddressMemoryIterator<IntType> &it1, const ConstVirtualAddressMemoryIterator<IntType> &it2)
{
    return !it1.virtualAddress->equalTo(it2.virtualAddress.get());
}

//----------------------------------------------------------------------------


// VirtualAddressMemoryIterator
// ConstVirtualAddressMemoryIterator

template<typename IntType>
VirtualAddressMemoryIterator<IntType> makeLinearVirtualAddressMemoryIterator(Memory *pMemory, uint64_t addr, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss, const LinearAddressTraits &traits=LinearAddressTraits{})
{
    auto la = LinearAddress(addr, uint64_t(sizeof(IntType)), traits);
    return VirtualAddressMemoryIterator<IntType>(pMemory, la.clone(), memoryOptionFlags);
}

template<typename IntType>
ConstVirtualAddressMemoryIterator<IntType> makeLinearConstVirtualAddressMemoryIterator(const Memory *pMemory, uint64_t addr, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss, const LinearAddressTraits &traits=LinearAddressTraits{})
{
    auto la = LinearAddress(addr, uint64_t(sizeof(IntType)), traits);
    return ConstVirtualAddressMemoryIterator<IntType>(pMemory, la.clone(), memoryOptionFlags);
}

template<typename IntType>
VirtualAddressMemoryIterator<IntType> makeSegmentedVirtualAddressMemoryIterator(Memory *pMemory, uint64_t seg, uint64_t offs, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss, const SegmentedAddressTraits &traits=SegmentedAddressTraits{})
{
    auto sa = SegmentedAddress(seg, offs, uint64_t(sizeof(IntType)), traits);
    return VirtualAddressMemoryIterator<IntType>(pMemory, sa.clone(), memoryOptionFlags);
}

template<typename IntType>
ConstVirtualAddressMemoryIterator<IntType> makeSegmentedConstVirtualAddressMemoryIterator(const Memory *pMemory, uint64_t seg, uint64_t offs, MemoryOptionFlags memoryOptionFlags=MemoryOptionFlags::errorOnAddressWrap | MemoryOptionFlags::errorOnHitMiss, const SegmentedAddressTraits &traits=SegmentedAddressTraits{})
{
    auto sa = SegmentedAddress(seg, offs, uint64_t(sizeof(IntType)), traits);
    return ConstVirtualAddressMemoryIterator<IntType>(pMemory, sa.clone(), memoryOptionFlags);
}







//----------------------------------------------------------------------------

} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/marty_mem.h"

