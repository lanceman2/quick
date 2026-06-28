#define _GNU_SOURCE
#include <link.h>

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdatomic.h>
#include <ctype.h>


#include <debug.h>


///////////////////////////////////////////////////////////////////////
// CONFIGURATION
///////////////////////////////////////////////////////////////////////
//
// Add USER_PREFIX string to the start of the spew.
// Example: if USER_PREFIX is "QS_"  QS_ERROR will be printed in place of
// ERROR for a call to ERROR("...").  Leave unset if no prefix is
// wanted.
//#define USER_PREFIX  ""
//
//
// Spew level environment variable.  Leave unset if no environment
// variable is wanted.  If the environment variable is set the level
// gotten from it will override the level set by all other means, except
// it can't make it spew if the compiled in COMPILED_SPEW_LEVEL is below
// it.
//
// If you want the USER_PREFIX added you need to do that explicitly
// here.

#ifndef SPEW_LEVEL_ENV
// Comment this line out to not use at compile time or to set using a
// compiler command line option like:
// -DSPEW_LEVEL_ENV=SPEW_LEVEL
#  define SPEW_LEVEL_ENV "SPEW_LEVEL"
#endif

#ifndef SPEW_COLOR_ENV
// Comment this line out to not use at compile time or to set using a
// compiler command line option like:
// -DSPEW_COLOR_ENV=SPEW_COLOR
#  define SPEW_COLOR_ENV "SPEW_COLOR"
// The program user may turn on color spew with:
// % SPEW_COLOR=y program_name
#endif
//
//
// Default to turn on ANSI escape sequences.  Example: prints red ERROR
// prefix with color on for a call to ERROR("my error");
//
//  0 = off, 1 = on, 2 = auto
static atomic_uint color = 2;

// https://en.wikipedia.org/wiki/ANSI_escape_code
//
static const char *ttyColors[] = { 
//      level:      0    1     2      3      4     5
//                  n/a  red   yellow green  blue  none
//                  NONE ERROR WARN   NOTICE INFO  DEBUG
                    "0", "31", "93",  "32",  "34", "0" };


// Also you can set CPP macros when compiling a file that includes
// debug.h:
//
//   SPEW_LEVEL_DEBUG  - make DSPEW() exist in the file
//   SPEW_LEVEL_INFO   - make INFO() exist in the file
//   SPEW_LEVEL_NOTICE - make NOTICE() exist in the file
//   SPEW_LEVEL_WARN   - make WARN() exist in the file
//   SPEW_LEVEL_ERROR  - make ERROR() exist in the file
//   SPEW_LEVEL_NONE   - make none of the above
//   DEBUG             - make DASSERT() exist
//
///////////////////////////////////////////////////////////////////////
// END CONFIGURATION
///////////////////////////////////////////////////////////////////////


#ifndef SYS_gettid
#  error "SYS_gettid unavailable on this system"
#endif

#include "./debug.h"


// When this file was compiled the spew level was:
#ifdef SPEW_LEVEL_DEBUG
#  define COMPILED_SPEW_LEVEL 5
#endif
#ifndef COMPILED_SPEW_LEVEL
#  ifdef SPEW_LEVEL_INFO
#    define COMPILED_SPEW_LEVEL 4
#  endif
#endif
#ifndef COMPILED_SPEW_LEVEL
#  ifdef SPEW_LEVEL_NOTICE
#    define COMPILED_SPEW_LEVEL 3
#  endif
#endif
#ifndef COMPILED_SPEW_LEVEL
#  ifdef SPEW_LEVEL_WARN
#    define COMPILED_SPEW_LEVEL 2
#  endif
#endif
#ifndef COMPILED_SPEW_LEVEL
#  ifdef SPEW_LEVEL_ERROR
#    define COMPILED_SPEW_LEVEL 1
#  endif
#endif
#ifndef COMPILED_SPEW_LEVEL
#  define COMPILED_SPEW_LEVEL 0
#endif


// FIXME: Not portable:
#define  DIRSTR  "/"
#define  DIRCHR '/'


// We have use cases for this code without the db_lib_dir thingy.
//
// The library constructor and destructor are only needed to make
// db_lib_dir.  db_lib_dir needs to be calculated at runtime, and
// we get it at library startup, and free it in the destructor.
//
#ifndef NO_LIBDEBUG_CONSTRUCTOR


const char *db_lib_dir = 0;


static int dl_callback(struct dl_phdr_info *info,
                        size_t size, void *data) {

    const char *LibName = DIRSTR "libdebug.so";

    char *libDir = 0;
    size_t LibNameLen = strlen(LibName);

    // The path must include this string.  If someone changes the
    // filename, LibName, in the source and installation, this will be
    // broken; lots of things will be broken; like the ability to load
    // core block modules.
    const size_t len = strlen(info->dlpi_name);
    if(len < LibNameLen) return 0;
    const char *end = info->dlpi_name + len - LibNameLen;

    for(const char *c = info->dlpi_name; c <= end; ++c) {
        if(0 == strncmp(c, LibName, LibNameLen)) {
            libDir = realpath(info->dlpi_name, 0);
            ASSERT(libDir);
            // A BUG in realpath() sets errno
            errno = 0;
            break;
        }
    }

    if(libDir) {
        // Better be a full path.
        ASSERT(*libDir == DIRCHR);
        char *s;
        for(s = libDir + strlen(libDir) - 1; *s != DIRCHR; --s)
            *s = '\0';
        // Remove the '/'.
        *s = '\0';
        // This extra const-ifying may save us some pain in the long run.
        // If some code tries to change it, at least they get a compile
        // error.
        db_lib_dir = (const char *) libDir;
        return 1; // done
    }

    return 0;
}

static void __attribute__ ((constructor)) constructor(void);

static void constructor(void) {

    dl_iterate_phdr(dl_callback, 0);

    ASSERT(db_lib_dir, "Can't find the libdebug.so library path");
    DSPEW("db_lib_dir=\"%s\"", db_lib_dir);
}

static void __attribute__ ((destructor)) destructor(void);

static void destructor(void) {

    DSPEW("Cleaning up");

    if(db_lib_dir) {
        DZMEM((void *) db_lib_dir, strlen(db_lib_dir));
        free((void *) db_lib_dir);
    }
}


#endif // #ifndef NO_LIBDEBUG_CONSTRUCTOR


// For debugging this code.
//#define SPEW()   fprintf(stderr," -------------- %s():%d  errno=%d\n", __func__, __LINE__, errno)


// LEVEL may be debug, info, notice, warn, error, and
// off which translates to: 5, 4, 3, 2, 1, and 0 respectively.
//
// We need spewLevel to be thread safe so we make spewLevel atomic.  Note
// we only set it once and get it once in this file and it's not exposed
// to any other code.
static atomic_uint spewLevel = COMPILED_SPEW_LEVEL;


int db_get_compiled_spew_level(void) {
    // Returns the compile time spew level.
    return COMPILED_SPEW_LEVEL;
}

int db_get_spew_level(void) {
    return spewLevel;
}


// This is where the user can quiet down the code in the library, assuming
// that the library was not quiet already.
void db_set_spew_level(int level) {
    if(level > 5) level = 5;
    else if(level < 0) level = 0;
    spewLevel = level;
    DSPEW("Spew level set to %d", level);
}


#define BUFLEN  1024


// Fixes BUG.  isatty() was setting errno for the case when _db_spew() was
// called in "meson test".  "meson test" sets up a different kind of file
// stream to collect test logging data.
//
// I don't think that fprintf() and family set errno on error; neither
// does getenv().
//
static inline int Isatty(FILE *f) {

    if(!f) return 0;

    int errno_save = errno;
    errno = 0;

    int fd = fileno(f);
    if(fd <= -1) goto finish;
    if(isatty(fd)) return 1;

finish:

    errno = errno_save;
    return 0;
}

// in-lining vspew() with inline may make debugging code a little harder.
//
// pre = "ERROR: ", "WARN: ", "NOTICE: ", "INFO: ", or "DEBUG: "
//
static void vspew(int errn, const char *pre, const char *file,
        int line, const char *func, const char *fmt, va_list ap, int level) {

    // TODO: What the hell good is buffer when SPEW_FILE is 0?

    // We try to buffer this "spew" so that prints do not get intermixed
    // with other prints in multi-threaded programs.
    char buffer[BUFLEN];
    int len = 0;

    bool isColor = 0;
    switch(color) {
        case 1:
            isColor = 1;
            break;
        case 2:
            if(color >= 2 && SPEW_FILE && Isatty(SPEW_FILE))
                isColor = 1;
            break;
        default:
    }

    if(isColor)
        // https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences
        len += snprintf(&buffer[len], BUFLEN, "\033[%s;1;7m", ttyColors[level]);
#ifdef USER_PREFIX
    len += snprintf(&buffer[len], BUFLEN, "%s", USER_PREFIX);
#endif
    if(strlen(pre))
        len += snprintf(&buffer[len], BUFLEN, "%s", pre);
    if(isColor)
        len += snprintf(&buffer[len], BUFLEN, "\033[0m");

    if(errn) {

        // TODO: Looks like strerror_r(3) is broken on my system,
        // sys_errlist(3) is deprecated, and strerror(3) is not thread
        // safe.  I'm fucked, there is no easy thread safe way to get the
        // system error string.

        // TODO: very Linux specific code here:
        len += snprintf(&buffer[len], BUFLEN,
                " %s:%d:pid=%u:%zu %s():errno=%d:%s: ",
                file, line,
                getpid(), syscall(SYS_gettid), func,
                errn, strerror(errn) /* How the fuck can they make this
                                        not thread safe */);
    } else
        len += snprintf(&buffer[len], BUFLEN, " %s:%d:pid=%u:%zu %s(): ",
                file, line,
                getpid(), syscall(SYS_gettid), func);

    if(len < 10 || len > BUFLEN - 40) {
        //
        // This should not happen.

        //
        // Try again without stack buffering
        //
        if(SPEW_FILE && level) {
            if(errn) {
                fprintf(SPEW_FILE, "%s%s:%d:pid=%u:%zu %s():errno=%d:%s: ",
                    pre, file, line,
                    getpid(), syscall(SYS_gettid), func,
                    errn, strerror(errn));
            } else
                fprintf(SPEW_FILE, "%s%s:%d:pid=%u:%zu %s(): ",
                        pre, file, line,
                        getpid(), syscall(SYS_gettid), func);
        }

        int ret;
        ret = vsnprintf(&buffer[len], BUFLEN - len,  fmt, ap);
        if(ret > 0) len += ret;
        if(len + 1 < BUFLEN) {
            // Add newline to the end.
            buffer[len] = '\n';
            buffer[len+1] = '\0';
        }
        return;
    }

    int ret;
    ret = vsnprintf(&buffer[len], BUFLEN - len,  fmt, ap);

    if(ret > 0) len += ret;
    if(len + 1 < BUFLEN) {
        // Add newline to the end.
        buffer[len] = '\n';
        buffer[len+1] = '\0';
    }

    if(SPEW_FILE && level)
        fputs(buffer, SPEW_FILE);
}


void _db_spew(uint32_t levelIn, int errn,
        const char *pre, const char *file,
        int line, const char *func,
        const char *fmt, ...) {

#ifdef SPEW_LEVEL_ENV
    // TODO: Maybe do this just the first spew() call.
    //
    char *env = getenv(SPEW_LEVEL_ENV);
    if(env && env[0]) {
        char val = env[0];
        // Remove proceeding spaces:
        while(*env && isspace(val)) val = *env++;
        if(val <= '9') {
            if(val < '0')
                spewLevel = 0;
            else if(val > '5')
                spewLevel = 5;
            else
                spewLevel = val - '0';
        } else {
            switch(val) {
                case 'E': // Error
                case 'e': // error
                    spewLevel = 1;
                    break;
                case 'W': // Warn
                case 'w': // warn
                    spewLevel = 2;
                    break;
                case 'N': // Notice
                case 'n': // notice
                    spewLevel = 3;
                    break;
                case 'I': // Info
                case 'i': // info
                    spewLevel = 4;
                    break;
                case 'D': // Debug
                case 'd': // debug
                    spewLevel = 5;
                    break;
                default:
                    spewLevel = COMPILED_SPEW_LEVEL;
            }
        }
    }
#endif

#ifdef SPEW_COLOR_ENV
    // TODO: Maybe do this just the first spew() call.
    //
#  ifndef SPEW_LEVEL_ENV
    // Declare env just once:
    char *
#  endif
    env = getenv(SPEW_COLOR_ENV);
    if(env && *env) {
        char val = *env++;
        // Remove proceeding spaces:
        while(*env && isspace(val)) val = *env++;
        if(val <= '0')
            color = 0;
        else if(val >= '2' && val <= '9')
            color = 2;
        else
            switch(val) {
                case 'A': // Automatic
                case 'a': // automatic
                    color = 2;
                    break;
                case 'Y': // Yes
                case 'y': // yes
                case 'T': // True
                case 't': // true
                case '1': // 1
                     color = 1;
                    break;
                case 'O': // On or Off
                case 'o': // on or off
                    val = *env++; // val == '\0' is fine too.
                    switch(val) {
                        case 'N': // ON or oN
                        case 'n': // on or On
                            color = 1;
                            break;
                        default: // Off, off, or Owhatever
                            color = 0;
                            break;
                    }
                    break;
                default:
                    // No, no, False, false, stuff, or whatever
                    color = 0;
            }
    }
#endif
 
    if(levelIn > spewLevel)
        // The spew level in is larger (more verbose) than one we let
        // spew.
        return;

    va_list ap;
    va_start(ap, fmt);
    vspew(errn, pre, file, line, func, fmt, ap, levelIn);
    va_end(ap);
}



// The library or program using this DEBUG package may set this
void (*db_assert_action)(const char *file,
        int lineNum, const char *func) = 0;


// This function is used in public CPP macro wrappers, but
// is not expected to be used directly by API users.
//
void _db_assert(const char *file,
        int lineNum, const char *func) {

    pid_t pid;
    pid = getpid();
    if(db_assert_action)
        // We call the users assert action.  If it does not exit that's
        // okay, we'll just fall into the default behavior.
        db_assert_action(file, lineNum, func);
#ifdef ASSERT_ACTION_EXIT
    fprintf(SPEW_FILE, "Will exit due to error\n");
    exit(1); // atexit() calls are called
    // See `man 3 exit' and `man _exit'
#else // ASSERT_ACTION_SLEEP
    int i = 1; // User debugger controller, unset to effect running code.
    fprintf(SPEW_FILE, "  Consider running: \n\n  gdb -pid %u\n\n  "
        "pid=%u:%zu will now SLEEP ...\n", pid, pid, syscall(SYS_gettid));
    while(i) { sleep(1); }
#endif
}
