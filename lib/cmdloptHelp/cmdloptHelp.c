/* This file and the program that it makes has to do with command-line
   options for the program that is installed as from ../../../bin/.

   This program is not a regular user program, but it is used by the
   command-line program to keep the command-line options and their
   documentation consistent by having the options and the documentation of
   them in one place, here in this file.  Also keeping this separate from
   the program source code makes the compiled program a little smaller.
   We don't have all these large strings in the program.

   This program will make the building of the project fail if options that
   are defined in the file are found to be inconsistent at project build
   time, and that's a good thing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <signal.h>

#include "../../../include/debug.h"
#include "cmdloptHelp.h"


/*  These are description special sequences that make HTML markup that is
    in the description.  Of course they mean something else for other
    output formats.  This special markup is only used in this file, and
    none of this special markup "bleeds" out of this file, just the
    standard markup comes out; be it HTML or simple ASCII text.  TODO: we
    support outputting man page markup too.

      "**" = start list <ul> 
      "##"  = <li>
      "&&" = end list   </ul>
  
      "  " = "&nbsp; "

      "--*"  =  "<a href="#name">--*</a>"  where --* is a long_op

      "::"   = "<span class=code>"  and will not add '\n' until @@
      "@@"   = "</span>"
 */

/* The program has an auto-generated companion
   options.h header file that is also generated from this file.
   Changing some of the parameters and definitions in this file will
   effect/configure the command-line program, and other
   programs that use the program.


   Thoughts: Should we just put this in an ASCII text file and write
   another program, or use another program, that parses it?  We think
   compiling the documentation file into the parser is a much simpler
   solution.  The dis-advantages are:

     1. that we must format the document strings as C strings,
     2. this method is not common, and
     3. the source to the package may be very slightly larger.  I doubt it
        is.

   The advantage with this method is:

     1. it's simple,
     2. it automatically checks that short arguments are not repeated,
     3. it can be one source file less,
     4. it's less easy to break,
     5. installers are practically required to install this documentation.
        You can't compile the program without it,
     6. the program does not need to link to more shit; given
        we require the program to be self documenting,
     7. the program ends up smaller,
     8. the program runs faster,
     9. the installed package source ends up smaller, because
        all these long comments do not get compiled into the installed code,
    10. the list package prerequisites ends up smaller, and
    11. I'm doing the work not you; so piss off.

   Oh yes, and it's stupid ass simple for anyone that can program in C.
*/


#define STRING(a)   _STR(a)
#define _STR(a)     #a


#ifndef PROG
#  define PROG   "PROGRAM"
#endif


static void
catcher(int sig) {
    ASSERT(0, "Caught signal number %d", sig);
}



// So by putting these options descriptions in this separate program we
// can generate:
//
//   1. HTML documentation,
//   2. the --help ASCII text from running 'PROG --help', and
//   3. man pages
//
// from this one source, and all the output forms stay consistent, because
// they all come from this one source file.
//
// This is so much easier than using a parser and some bullshit document
// format.  And it does not bloat any of the code with a crap load of
// strings, except this program, which does not matter.  It's just this
// one program that holds the strings.  The programs that use this are
// different programs that run this program.  It's stupid simple.
//
// The code below will check for duplicate options, but it will not sort
// these; so sort them now here:


// Kind-of like:
// https://stackoverflow.com/questions/18783988/how-to-get-windows-size-from-linux
//
static int getCols(const int fd) {
    struct winsize sz;
    int result;

    do {
        result = ioctl(fd, TIOCGWINSZ, &sz);
    } while (result == -1 && errno == EINTR);

    if(result == -1)
        return 76; // default width
    return sz.ws_col;
}


static inline int NSpaces(int n, int c) {
    int ret = n;
    if(n < 1) return 0;
    while(n--) putchar(c);
    return ret;
}

// examples:  str="  hi ho"  returns 4
//            str=" hi ho"   returns 3
//            str="hi ho"    returns 2
static inline int GetNextWordLength(const char *str) {

    int n = 0;

    if(*str == '\n')
        return n;

    while(*str == ' ') {
        ++n;
        ++str;
    }
    while(*str && *str != ' ' && *str != '\n') {

        // skip special chars
        if(*str == '*' && *(str+1) == '*') {
            str += 2;
            continue;
        } else if(*str == '#' && *(str+1) == '#') {
            str += 2;
            continue;
        } else if(*str == '&' && *(str+1) == '&') {
            str += 2;
            continue;
        }

        ++n;
        ++str;
    }
    return n;
}

// types of file output
#define HTML  (0)
#define TXT   (1)


static inline int PutNextWord(const char **s, int type) {

    int n = 0;
    const char *str = *s;

    while(*str == ' ') {
        putchar(*str);
        ++n;
        ++str;
    }
    while(*str && *str != ' ' && *str != '\n') {

        // replace special chars
        if(*str == '*' && *(str+1) == '*') {
            str += 2;
            if(type == HTML) {
                printf("<ul>\n");
            }
            continue;
        } else if(*str == '#' && *(str+1) == '#') {
            str += 2;
            if(type == HTML)
                printf("  <li>");
            continue;
        } else if(*str == '&' && *(str+1) == '&') {
            str += 2;
            if(type == HTML)
                printf("</ul>");
            continue;
        }

        putchar(*str);
        ++n;
        ++str;
    }

    *s = str;

    return n;
}


static void
printHtml(const char *s, int s1, int s2) {

    while(*s) {
    
        int n = 0;
        printf("<p>\n");

        while(*s) {

            // add desc up to length s2
            while(*s && (n + GetNextWordLength(s)) <= s2) {
                if(*s == '\n') { ++s; break; }
                n += PutNextWord(&s, HTML);
            }
            printf("\n");
            n = 0;
            if(*s == '\n' || *s == '\0') {
                printf("</p>\n");
                if(*s) ++s;
                break;
            }
        }
    }
}


static void
printParagraphs(const char *s, int s1, int s2, int count) {

    int n = count;
    bool starting;

    while(*s) {

        starting = true;

        // add desc up to length s2
        while(*s && (((n + GetNextWordLength(s)) <= s2) || starting)) {
            if(*s == '\n') { ++s; break; }
            starting = false;
            n += PutNextWord(&s, TXT);
        }

        printf("\n");

        if(*s == '\0') break;

        // Start next row.
        n = 0;
        n += NSpaces(s1, ' ');

        // Do not start with a single space.
        if(*s == ' ' && *(s+1) != ' ') ++s;
    }
}


static inline bool IfBase62(int a) {

    return (a >= '0' && a <= '9') ||
         (a >= 'A' && a <= 'Z') ||
         (a >= 'a' && a <= 'z');
}


static void
printDescription(const struct opts *opt, int s0, int s1, int s2) {

    const char *s = opt->description;
    int n = 0;

    // start with "    --long-arg ARG  "
    n += NSpaces(s0, ' ');
    if(IfBase62(opt->short_op))
        n += printf("%s|-%c", opt->long_op, opt->short_op);
    else
        n += printf("%s", opt->long_op);

    if(opt->arg)
        n += printf(" %s", opt->arg);
    n += printf("  ");
    n += NSpaces(s1 - n, ' ');

    printParagraphs(s, s1, s2, n);
}



static char *ArgLowerToUpper(const char *str) {

#define LEN 256
    static char result[LEN+1];
    char *r = result;
    size_t len_in = strlen(str);

    if(len_in > LEN) {
        ERROR("Fix this code");
        exit(1);
    }
    const size_t delta = 'a' - 'A';

    while(*str) {
        if(*str >= 'a' && *str <= 'z')
            *r = *str - delta;
        else if(*str == '-')
            *r = '_';
        else {
            fprintf(stderr, "Fix this code: %s:%d\n", __FILE__, __LINE__);
            exit(1);
        }
        r++;
        str++;
    }
    *r = '\0';

    return result;     
}


static const char *usage = 0;
static char * (*get_post_help)(void) = 0;
struct opts *opts = 0;



void cmdlopt_init(const char *usage_in,
        char * (*get_post_help_in)(void),
        struct opts *opts_in) {

    DASSERT(usage_in);
    DASSERT(get_post_help_in);
    DASSERT(opts_in);

    usage = usage_in;
    get_post_help = get_post_help_in;
    opts = opts_in;
}


int Main(int argc, char **argv) {
   
    signal(SIGSEGV, catcher);

    {
        ssize_t n = 2;
        if(argc > 1)
            n = strlen(argv[1]);

        if(argc != 2 || n != 2 ||
                argv[1][0] != '-' || 
                (argv[1][1] != 'c' &&
                 argv[1][1] != 'H' &&
                 argv[1][1] != 'i' &&
                 argv[1][1] != 'l' &&
                 argv[1][1] != 'o' && argv[1][1] != 'O' &&
                 argv[1][1] != 's' &&
                 argv[1][1] != 'u' &&
                 argv[1][1] != 'w')
                || argv[1][2] != '\0'
        ) {
            // Usage for the PROG "usage program".  Wow: that
            // actually makes sense.
            //
            printf("    Usage: %s "
                    "[ -c | -H | -i | -l | -o | -O | -s | -u | -w ]\n"
                "\n"
                "    Generate HTML, text, and C code that is related\n"
                "  to the program " PROG ".\n"
                "\n"
                "    This program helps us keep documentation and code\n"
                "  consistent, by putting the command-line options\n"
                "  documentation and code in one file.\n"
                "  Returns 0 on success and 1 on error unless stated\n"
                "  otherwise.  This program always prints to stdout\n"
                "  when successful.\n"
                "\n"
                " -----------------------------------------\n"
                "              OPTIONS\n"
                " -----------------------------------------\n"
                "\n"
                "    -c  print the C code of the argument options\n"
                "\n"
                "    -H  print --help text for " PROG " and exit 0\n"
                "\n"
                "    -i  print intro in HTML\n"
                "\n"
                "    -l  print all long options\n"
                "\n"
                "    -o  print HTML options table\n"
                "\n"
                "    -O  print all options with a space between\n"
                "\n"
                "    -s  print all the " PROG " program short options\n"
                "\n"
                "    -u  print just the " PROG " program usage\n"
                "\n"
                "    -w  print all options without ARGS in a map\n"
                "\n",
                argv[0]);
            return 1;
        }
    }

    // start and stop char positions:
    const int s0 = 3;
    int s1 = 18;
    int s2; // tty width or default width


    struct opts *opt = opts; // opt is an iterator

    while(opt->description) {
        struct opts *opt2 = opt+1;
        while(opt2->description) {
            //
            // Check for duplicate argument options any time this program
            // runs.
            //
            if(strcmp(opt2->long_op, opt->long_op) == 0) {
                fprintf(stderr, "ERROR: We have at least 2 "
                        "long %s " PROG " argument "
                        "options in " __FILE__ "\n", opt2->long_op);
                return 1; // fail
            }
            if(IfBase62(opt->short_op) &&
                    opt2->short_op == opt->short_op) {
                fprintf(stderr,
                        "ERROR there are at least two options with "
                        "short option -%c in %s.in\n",
                        opt2->short_op, __BASE_FILE__);
                return 1;
            }
            ++opt2;
        }
        ++opt;
    }

    opt = opts;


    switch(argv[1][1]) {

        case 'c':
            printf("// This is a generated file\n\n");

            while(opt->description) {
                // We had #define instead of static const char *
                // Is avoiding CPP macros a good thing?  Ya, maybe.
                // TODO: Extend to cases that are not ASCII, i.e. max
                // value 256 not just 64 (or like) possibilities.
                printf("#define %s  (%d) ",
                        ArgLowerToUpper(opt->long_op + 2), opt->short_op);
                if(IfBase62(opt->short_op))
                    printf("  // '%c'", opt->short_op);
                printf("\n");
                ++opt;
            }
            printf("\n");

            // reset opt to loop again.
            opt = opts;

            printf(
                    "static const\n"
                    "struct opts options[] = {\n");
            while(opt->description) {
                printf("    { \"%s\", %d },",
                        opt->long_op + 2, opt->short_op);
                if(IfBase62(opt->short_op))
                    printf("  // '%c'", opt->short_op);
                printf("\n");

                ++opt;
            }
            printf("    { 0, 0 }\n};\n");
            return 0;

        case 'l':
            for(; opt->description; ++opt)
                printf(" %s\n", opt->long_op);
            return 0;

        case 'H': // print the --help
        case 'u':
            s2 = getCols(STDOUT_FILENO) - 1;
            if(s2 > 200) s2 = 200;
            if(s2 < 60) s2 = 60;

            printf("\n");
            printParagraphs(usage, s0, s2, 0);
            printf("\n");

            if(argv[1][1] == 'u') {
                // Just usage.
                printf("\n  all OPTIONS, long than short, are:\n\n");
                while((*opt).description) {
                    if(opt->arg) {
                        // Has argument sub-argument
                        printf(" %s %s\n", opt->long_op, opt->arg);
                        if(IfBase62(opt->short_op))
                            printf(" -%c %s\n", opt->short_op, opt->arg);
                    } else {
                        // No argument sub-argument
                        //
                        printf(" %s\n", opt->long_op);
                        if(IfBase62(opt->short_op))
                            printf(" -%c\n", opt->short_op);
                    }
                    printf("\n");
                    ++opt;
                }
                return 0;
            }

            putchar(' ');
            NSpaces(s2-1, '-');
            putchar('\n');
            NSpaces(s2/2 - 6, ' ');
            printf("OPTIONS\n");
            putchar(' ');
            NSpaces(s2-1, '-');
            printf("\n\n");

            while((*opt).description) {
                printDescription(opt, s0, s1, s2);
                printf("\n");
                ++opt;
            }
            printf("\n");
            {
                char *post_help = get_post_help();
                printParagraphs(post_help, s0, s2, 0);
                free(post_help);
            }
            printf("\n");
            return 0; // no error

        case 'i':
            printHtml(usage, 4, 76);
            return 0;

        case 'o':
            printf("<pre>\n");
            s2 = 80;
            printf("\n");
            while((*opt).description) {
                printDescription(opt, s0, s1, s2);
                printf("\n");
                ++opt;
            }
            printf("</pre>\n");
            return 0;

        case 'O': // Print all options with a space between

            printf("%s", opt->long_op);
            if(IfBase62(opt->short_op))
                printf(" -%c", opt->short_op);
            ++opt;
            while(opt->description) {
                printf(" %s", opt->long_op);
                if(IfBase62(opt->short_op))
                    printf(" -%c", opt->short_op);
                ++opt;
            }
            putchar('\n');
            return 0;

        case 's':

            while(opt->description) {
                if(IfBase62(opt->short_op))
                    printf("%c\n", opt->short_op);
                ++opt;
            }
            //putchar('\n');
            return 0;
    
        case 'w': // Print all options without ARGS

            {
                bool gotOne = false;

                // First long options
                if(opt->arg == 0) {
                    printf("[%s]=1", opt->long_op);
                    if(IfBase62(opt->short_op))
                        printf(" [-%c]=1", opt->short_op);
                    gotOne = true;
                }
                ++opt;
                while(opt->description) {
                    if(opt->arg == 0) {
                        if(gotOne)
                            putchar(' ');
                        else
                            gotOne = true;
                        printf("[%s]=1", opt->long_op);
                        if(IfBase62(opt->short_op))
                            printf(" [-%c]=1", opt->short_op);
                    }
                    ++opt;
                }
                putchar('\n');
                return 0;
            }
     }

    // This should not happen.
    return 1;
}
