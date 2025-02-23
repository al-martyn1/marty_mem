/*! \file
    \brief Исключения для marty::mem. Стиль именов - snake_case, так как мы наследуемся от стандартной библиотеки
 */

#pragma once

//----------------------------------------------------------------------------
#include "enums.h"
#include "utils.h"

//----------------------------------------------------------------------------
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// #include "marty_mem/exceptions.h"
// marty::mem::
namespace marty{
namespace mem{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#define MARTY_MEM_DECLARE_EXCEPTION_CLASS(cls, clsBase)                                  \
             class cls : public clsBase                                                  \
             {                                                                           \
             public:                                                                     \
                                                                                         \
                 cls(const std::string &what_arg) : clsBase(what_arg) {}                 \
                 cls(const char* what_arg       ) : clsBase(std::string(what_arg)) {}    \
                 cls(const cls &e)                : clsBase(e) {}                        \
                 cls& operator=(const cls& e)   { clsBase::operator=(e); return *this; } \
             }


//----------------------------------------------------------------------------
// class base_error : public std::runtime_error // exception
// {
//  
//   typedef std::runtime_error base_exception_t;
//  
// public:
//  
//     base_error(const std::string& what_arg) : base_exception_t(what_arg             ) {}
//     base_error(const char* what_arg       ) : base_exception_t(std::string(what_arg)) {}
//  
//     base_error(const base_error &e)
//     : base_exception_t(e)
//     {}
//  
//     base_error& operator=(const base_error& e) = delete;
//  
// }; // class base_error

MARTY_MEM_DECLARE_EXCEPTION_CLASS(base_error, std::runtime_error);
    MARTY_MEM_DECLARE_EXCEPTION_CLASS(invalid_value, base_error);
    MARTY_MEM_DECLARE_EXCEPTION_CLASS(unaligned_iterator, base_error);
    MARTY_MEM_DECLARE_EXCEPTION_CLASS(invalid_address_difference, base_error);
    MARTY_MEM_DECLARE_EXCEPTION_CLASS(incompatible_address_pointers, base_error);
    MARTY_MEM_DECLARE_EXCEPTION_CLASS(memory_access_error, base_error);
        MARTY_MEM_DECLARE_EXCEPTION_CLASS(access_denied           , memory_access_error);
        MARTY_MEM_DECLARE_EXCEPTION_CLASS(unassigned_memory_access, memory_access_error);
        MARTY_MEM_DECLARE_EXCEPTION_CLASS(unaligned_memory_access , memory_access_error);
        MARTY_MEM_DECLARE_EXCEPTION_CLASS(address_wrap            , memory_access_error);


// Кидает исключение, соответствующее коду ошибки. Если ошибки нет - не кидает
inline
void throwMemoryAccessError(MemoryAccessResultCode rc, const std::string &msg=std::string())
{
    switch(rc)
    {
        case MemoryAccessResultCode::invalid               : throw invalid_value(!msg.empty() ? msg : std::string("invalid MemoryAccessResultCode"));
        case MemoryAccessResultCode::accessGranted         : break;
        case MemoryAccessResultCode::accessDenied          : throw access_denied(!msg.empty() ? msg : std::string("access denied"));
        case MemoryAccessResultCode::unassignedMemoryAccess: throw unassigned_memory_access(!msg.empty() ? msg : std::string("unassigned memory access"));
        case MemoryAccessResultCode::unalignedMemoryAccess : throw unassigned_memory_access(!msg.empty() ? msg : std::string("unaligned memory access"));
        case MemoryAccessResultCode::addressWrap           : throw address_wrap(!msg.empty() ? msg : std::string("address/offset wrap occured"));
    }
}

inline
void throwMemoryAccessError(MemoryAccessResultCode rc, const char *msg)
{
    throwMemoryAccessError(rc, std::string(msg));
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/exceptions.h"


