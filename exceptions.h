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
        MARTY_MEM_DECLARE_EXCEPTION_CLASS(memory_fill_error       , memory_access_error);


inline
std::string getDefaultMemoryAccessErrorMessage(MemoryAccessResultCode rc)
{
    switch(rc)
    {
        case MemoryAccessResultCode::invalid               : return "MemoryAccessResultCode::invalid";
        case MemoryAccessResultCode::accessGranted         : return "access granted";
        case MemoryAccessResultCode::accessDenied          : return "access denied";
        case MemoryAccessResultCode::unassignedMemoryAccess: return "unassigned memory access";
        case MemoryAccessResultCode::unalignedMemoryAccess : return "unaligned memory access";
        case MemoryAccessResultCode::addressWrap           : return "address/offset wrap occured";
        case MemoryAccessResultCode::memoryFillError       : return "memory fill error";
        default: return "unknown MemoryAccessResultCode";
    }
}

inline
std::string getMemoryAccessErrorMessage(MemoryAccessResultCode rc, const std::string &customMsg=std::string())
{
    return !customMsg.empty() ? customMsg : getDefaultMemoryAccessErrorMessage(rc);
}

// Кидает исключение, соответствующее коду ошибки. Если ошибки нет - не кидает
inline
void throwMemoryAccessError(MemoryAccessResultCode rc, const std::string &msg=std::string(), std::string msgExtra=std::string())
{
    if (!msgExtra.empty())
        msgExtra = ": " + msgExtra;
    switch(rc)
    {
        case MemoryAccessResultCode::accessGranted         : break;

        case MemoryAccessResultCode::invalid               : throw invalid_value           (getMemoryAccessErrorMessage(rc, msg)+msgExtra);
        case MemoryAccessResultCode::accessDenied          : throw access_denied           (getMemoryAccessErrorMessage(rc, msg)+msgExtra);
        case MemoryAccessResultCode::unassignedMemoryAccess: throw unassigned_memory_access(getMemoryAccessErrorMessage(rc, msg)+msgExtra);
        case MemoryAccessResultCode::unalignedMemoryAccess : throw unassigned_memory_access(getMemoryAccessErrorMessage(rc, msg)+msgExtra);
        case MemoryAccessResultCode::addressWrap           : throw address_wrap            (getMemoryAccessErrorMessage(rc, msg)+msgExtra);
        case MemoryAccessResultCode::memoryFillError       : throw memory_fill_error       (getMemoryAccessErrorMessage(rc, msg)+msgExtra);

        default: return;
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


