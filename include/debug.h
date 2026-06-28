#ifndef __debug_h__
#define __debug_h__

/*

This file provides some CPP (C pre processor) macro debug functions.

At run-time activity of the macro functions DSPEW(), INFO(), NOTICE(),
WARN(), and ERROR() is determined by the current spew level;
that is if they exist at compile time.

This file is really not much code but is a powerful development tool like
assert(3).  See 'man 3 assert'.  This just adds spewing of line number,
name of function, and other handy information to the basic idea of libc's
assert(3).  For DEBUG mode builds: sprinkle DASSERT() into all your
functions, and zero all memory allocations before you free them.

Most developers have their own form of this crap and we understand it is
best to just use your own macro-ised form of fprintf(strerr, ...) that let
you watch your code run or fail to run correctly.

*/

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>


#ifdef debug_BUILD_LIB
// This is being compiled into a (software project) library.
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern
//#  define EXPORT extern
#else
// The library API user is using this including this header file.
#  define EXPORT extern
#endif

/*
///////////////////////////////////////////////////////////////////////////
   COMPILE TIME DEFINABLE SELECTOR MACROS make the follow macros come to
   life:
 
   DEBUG             -->  DASSERT()
   DEBUG             -->  DZMEM()

   SPEW_LEVEL_DEBUG  -->  DSPEW() INFO() NOTICE() WARN() ERROR()
   SPEW_LEVEL_INFO   -->  INFO() NOTICE() WARN() ERROR()
   SPEW_LEVEL_NOTICE -->  NOTICE() WARN() ERROR()
   SPEW_LEVEL_WARN   -->  WARN() ERROR()
   SPEW_LEVEL_ERROR  -->  ERROR()

   always on is      --> ASSERT() CHECK() RET_ERROR() ERROR()

   If a macro function is not live it becomes a empty macro with no C code.


///////////////////////////////////////////////////////////////////////////

   Setting SPEW_LEVEL_NONE with still have ASSERT() spewing and ERROR()
   will not spew.
*/


/*

This file along with the public debug.h and debug.c provides some CPP (C
pre processor) macro debug functions.

At run-time activity of the macro functions DSPEW(), INFO(), NOTICE(),
WARN(), and ERROR() is determined by the current spew level;
that is if they exist at compile time (given SPEW_LEVEL_* macros).

This file is really not much code but is a powerful development tool like
assert(3).  See 'man 3 assert'.  This just adds spewing of line number,
name of function, and other handy information to the basic idea of libc's
assert(3).  For DEBUG mode builds: sprinkle DASSERT() into all your
functions, and zero all memory allocations before you free them.

Most developers have their own form of this crap and we understand it is
best to just use your own macro-ised form of fprintf(strerr, ...) that let
you watch your code run or fail to run correctly.

*/


// Compiled in default spew file:
#define SPEW_FILE stderr


// Compiled in default, the environment can turn it off at run-time.
// but the function must by called and if() return; it's self off
//
// if say SPEW_LEVEL_INFO is set then all spews below INFO are removed
// from the compiled code.
//
#ifndef SPEW_LEVEL_DEBUG
#  define SPEW_LEVEL_DEBUG
#endif


// Other compiled-in CPP options see in debug/debug.c
//
//#define USER_PREFIX ""  
//#define SPEW_LEVEL_ENV "SPEW_LEVEL"
//#define SPEW_COLOR_ENV "SPEW_COLOR"

// The API user may define DEBUG or not.


#ifdef DEBUG
#  define DZMEM(x,size)  memset((x), 0, (size))
// Another way to test for bad/freed memory access:
//#  define DZMEM(x,size)  memset((x), 0xFFFFFFFF, (size))
#else
#  define DZMEM(x,size)  /* empty macro */
#endif

#ifdef __GNUC__
// We would like to be able to just call DSPEW() with no arguments
// which can make a zero length printf format.
#  pragma GCC diagnostic ignored "-Wformat-zero-length"
#endif


#ifdef __cplusplus
extern "C" {
#endif



EXPORT
void (*db_assert_action)(const char *file,
        int lineNum, const char *func);

EXPORT
void _db_spew(uint32_t level, int errn, const char *pre,
        const char *file, int line, const char *func,
        const char *fmt, ...)
#ifdef __GNUC__
        // check printf format errors at compile time:
        __attribute__ ( ( format (printf, 7, 8 ) ) )
#endif
        ;

EXPORT
void _db_assert(const char *file,
        int lineNum, const char *func);


EXPORT
int db_get_compiled_spew_level(void);

// levels = 0 none, 1 error, 2 warn, 3 notice, 4 info, 5 debug
EXPORT
int db_get_spew_level(void);

EXPORT
void db_set_spew_level(int level);

EXPORT
const char *db_lib_dir;


// This CPP macro function CHECK() is just so we can call most pthread_*()
// (pthread_mutex_init() for example) and maybe other functions that
// return 0 on success and an int error number on failure, and asserts on
// failure printing errno (from ASSERT()) and the return value.  This is
// not so bad given this does not obscure what is being run.  Like for
// example:
//
//   CHECK(pthread_mutex_lock(&s->mutex));
//
// You can totally tell what that is doing.  One line of code instead of
// three.  Note (x) can only appear once in the macro expression,
// otherwise (x) could get executed more than once if it was listed more
// than once.
//
// CHECK() is not a debugging thing; it's inserting the (x) code every
// time.  And the same goes for ASSERT(); ASSERT() is not a debugging
// thing either, it is the coder being to lazy to recover from a large
// number of failure code paths.  If malloc(10) fails we call ASSERT.
// If pthread_mutex_lock() fails we call ASSERT. ...
//
#define CHECK(x) \
    do { \
        int ret = (x); \
        ASSERT(ret == 0, #x "=%d FAILED", ret); \
    } while(0)


// This seems to be an often repeated pattern.  They also seem to have
// crap like this in GTK+, not that that's necessarily a good thing.
//
// This acts like an ASSERT() in that the val arg needs to be true.
//
// If val is not true return ret, else do not return.
//
#define RET_ERROR(val, ret, ...) \
    do {\
        if(!((bool) (val))) {\
            ERROR("" __VA_ARGS__);\
            return ret;\
        }\
    }\
    while(0)

// Like above but with ERROR -> WARN.
//
// If val is not true return ret, else do not return.
//
#define RET_WARN(val, ret, ...) \
    do {\
        if(!((bool) (val))) {\
            WARN("" __VA_ARGS__);\
            return ret;\
        }\
    }\
    while(0)


#  define _SPEW(level, errn, pre, fmt, ... )\
     _db_spew(level, errn, pre, __BASE_FILE__, __LINE__,\
        __func__, fmt, ##__VA_ARGS__)


// It's nice to see that it is ASSERT() or DASSERT() as it is in the code;
// hence we pass fname as ASSERT or DASSERT.
#  define DO_ASSERT(fname, val, ...) \
    do {\
        if(!((bool) (val))) {\
            _SPEW(1, errno, #fname"("#val") failed:", "" __VA_ARGS__);\
            _db_assert(__BASE_FILE__, __LINE__, __func__);\
        }\
    }\
    while(0)


#  define ASSERT(val, ...)   DO_ASSERT(ASSERT, val, ##__VA_ARGS__)




#ifdef DEBUG
#  define DASSERT(val, ...)  DO_ASSERT(DASSERT, val, ##__VA_ARGS__)
#else
#  define DASSERT(val, ...)  /*empty macro*/
#endif



// The highest verbosity must cause all lower verbosities to exist:
/////////////////////////////////////////////////////////////////////////////
// We let the highest verbosity macro flag win.
//
#ifdef SPEW_LEVEL_DEBUG
#  ifndef SPEW_LEVEL_INFO
#    define SPEW_LEVEL_INFO
#  endif
#endif
#ifdef SPEW_LEVEL_INFO
#  ifndef SPEW_LEVEL_NOTICE
#    define SPEW_LEVEL_NOTICE
#  endif
#endif
#ifdef SPEW_LEVEL_NOTICE
#  ifndef SPEW_LEVEL_WARN
#    define SPEW_LEVEL_WARN
#  endif
#endif
#ifdef SPEW_LEVEL_WARN
#  ifndef SPEW_LEVEL_ERROR
#    define SPEW_LEVEL_ERROR
#  endif
#endif
//
#ifdef SPEW_LEVEL_ERROR
#  ifdef SPEW_LEVEL_NONE
#    undef SPEW_LEVEL_NONE
#  endif
#else
#  ifndef SPEW_LEVEL_NONE
#    define SPEW_LEVEL_NONE // must be set if no other is set.
#  endif
#endif



#ifdef SPEW_LEVEL_NONE
#define ERROR(...) _SPEW(0, errno, "ERROR:", "" __VA_ARGS__)
#else
#define ERROR(...) _SPEW(1, errno, "ERROR:", "" __VA_ARGS__)
#endif

#ifdef SPEW_LEVEL_WARN
#  define WARN(...) _SPEW(2, errno, "WARN:", "" __VA_ARGS__)
#else
#  define WARN(...) /*empty macro*/
#endif 

#ifdef SPEW_LEVEL_NOTICE
#  define NOTICE(...) _SPEW(3, errno, "NOTICE:", "" __VA_ARGS__)
#else
#  define NOTICE(...) /*empty macro*/
#endif

#ifdef SPEW_LEVEL_INFO
#  define INFO(...)   _SPEW(4, 0, "INFO:", "" __VA_ARGS__)
#else
#  define INFO(...) /*empty macro*/
#endif

#ifdef SPEW_LEVEL_DEBUG
#  define DSPEW(...)  _SPEW(5, 0, "DEBUG:", "" __VA_ARGS__)
#else
#  define DSPEW(...) /*empty macro*/
#endif



#ifdef __cplusplus
}
#endif



#undef EXPORT

#endif // #ifndef __debug_h__
