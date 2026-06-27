#include <stdio.h>
#include <dlfcn.h>

#include "../../include/debug.h"


int main( int argc, char **argv ) {

    const size_t Len = 256;
    char path[Len];
    snprintf(path, Len, "%s/libdebug.so", db_lib_dir);

    ASSERT(strlen(path) > 4);

    void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    dlclose(handle);

    return 0;
}
