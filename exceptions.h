/*! \file
    \brief Исключения для marty::mem. Стиль именов - snake_case, так как мы наследуемся от стандартной библиотеки
 */

#pragma once

//----------------------------------------------------------------------------
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
class base_error : public std::runtime_error // exception
{

  typedef std::runtime_error base_exception_t;

public:

    base_error(const std::string& what_arg) : base_exception_t(what_arg             ) {}
    base_error(const char* what_arg       ) : base_exception_t(std::string(what_arg)) {}

    base_error(const base_error &e)
    : base_exception_t(e)
    {}

    base_error& operator=(const base_error& e) = delete;

}; // class base_error

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
class unaligned_access : public base_error
{

public:

    // const std::string fileName;

    unaligned_access(const std::string &what_arg)
    : base_error(what_arg)
    {}

    unaligned_access(const unaligned_access &e)
    : base_error(e)
    {}

    unaligned_access& operator=(const unaligned_access& e) = delete;

};

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
} // namespace mem
} // namespace marty
// marty::mem::
// #include "marty_mem/exceptions.h"


