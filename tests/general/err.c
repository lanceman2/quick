
#include <stdio.h>
#include <err.h>
#include <errno.h>


int main(void) {

    errno = 23;

    // Testing this new BSD extension from the year 1993.  New to me at
    // the time I write this test code Sun Oct  4 01:40:20 PM EDT 2026.
    // I'm a little slow, or it took a while for GNU libc to get it added.
    //
    // This may replace some of the (non-standard) CPP macro spew crap
    // that I use all the time in my code.  Okay looks like it's not
    // so helpful.  My CPP macro spew functions beat it by far
    // (example: lib/libdebug.so, include/debug.h).
    err(5, "We got an error: %d", errno);

    return 0;
}

