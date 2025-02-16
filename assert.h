/*! \file
    \brief Asserts for marty::mem
 */

#pragma once

// #include "marty_mem/assert.h"


//----------------------------------------------------------------------------
#if defined __has_include
    #if __has_include(<assert.h>)
        #include <assert.h>
        #define MARTY_MEM_HAS_CRT_ASSERT
    #endif
#endif

//----------------------------------------------------------------------------
#if defined __has_include
    #if __has_include("umba/assert.h")
        #include "umba/assert.h"
        #define MARTY_MEM_HAS_UMBA_ASSERT
    #endif
#endif

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if !defined(MARTY_MEM_ASSERT)

    #if defined(UMBA_ASSERT)
    
        #define MARTY_MEM_ASSERT(expr)             UMBA_ASSERT(expr)
    
    #elif defined(MARTY_MEM_HAS_CRT_ASSERT)
    
        #define MARTY_MEM_ASSERT(expr)             assert(expr)
    
    #else /* no actually assertions */
    
        #define MARTY_MEM_ASSERT(expr)
    
    #endif

#endif

//----------------------------------------------------------------------------
#if !defined(MARTY_MEM_ASSERT_EX)

    #if defined(UMBA_ASSERT_EX)
    
        #define MARTY_MEM_ASSERT_EX(expr, msg)     UMBA_ASSERT_EX(expr, msg)
    
    #elif defined(MARTY_MEM_HAS_CRT_ASSERT)
    
        #define MARTY_MEM_ASSERT(expr, msg)        assert((msg, expr))
    
    #else /* no actually assertions */
    
        #define MARTY_MEM_ASSERT(expr)
    
    #endif

#endif

//----------------------------------------------------------------------------
#define MARTY_MEM_ASSERT_FAIL()           MARTY_MEM_ASSERT( 0 )
#define MARTY_MEM_ASSERT_FAIL_MSG(msg)    MARTY_MEM_ASSERT_EX( 0, msg )

//----------------------------------------------------------------------------


