// Not much of a test; just see if libdict.so links and runs with a
// compiled C program.  Can run with Valgrind too.

#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>

#include "../../include/debug.h"
#include "../../include/dict.h"


int main(void) {

    struct Dict *d = dict_create();

    const char *keys[] = { "h", "hello", "goodbye", 0 };

    uint32_t count = 1;

    for(const char *k = (void *) *keys; *k; k++)
        ASSERT(!dict_insert(d, (void *) k, strlen(k), (void *) (uintptr_t) (count++)));

    for(const char *k = (void *) *keys; *k; k++)
        ASSERT(dict_find(d, (void *) k, strlen(k)), "Can't find key=%s", k);

    for(const char *k = (void *) *keys; *k; k++)
        ASSERT(dict_remove(d, (void *) k, strlen(k)));

    dict_destroy(d);

    return 0;
}
