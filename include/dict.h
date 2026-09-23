#ifndef __DICT_H__
#define __DICT_H__

// This is a inline API wrapper of:
// https://raw.githubusercontent.com/armon/libart/
//
// A competing adaptive TRIE package.  This one did not preform as well
// in our benchmark, but may be better supported and documented:
// https://github.com/antirez/rax
//
// See also the benchmark code and scripts to run the benchmark:
// https://github.com/lanceman2/dictionary_benchmark


#include "art.h"
#include "debug.h"


#ifdef dict_BUILD_LIB
// This is being compiled into a (software project) library,
// libdict.so.
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern
#else
// The library API user is using this including this header file.
#  define EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

// We wrap all of these art functions with inline functions in this header
// file.
EXPORT int art_tree_init(art_tree *t);
EXPORT int art_tree_destroy(art_tree *t);
EXPORT void* art_insert(art_tree *t, const unsigned char *key,
        int key_len, void *value);
EXPORT void* art_delete(art_tree *t, const unsigned char *key,
        int key_len);
EXPORT void* art_search(const art_tree *t,
        const unsigned char *key, int key_len);



struct Dict {

    art_tree art;
};


static inline struct Dict *dict_create(void) {

    struct Dict *d;
    d = malloc(sizeof(*d));
    ASSERT(d, "malloc(%zu) failed", sizeof(*d));
    ASSERT(0 == art_tree_init(&d->art));

    return d;
}

static inline void dict_destroy(struct Dict *d) {

    // What does this do?
    ASSERT(0 == art_tree_destroy((void *) d));

    free((void *) d);
}

// return the old value or 0
static inline void *dict_insert(struct Dict *d,
        void *key,
        size_t len, void *data) {

    return art_insert((void *) d, key, len, data);
}

// return the value that was inserted or 0 if not found
static inline void *dict_remove(struct Dict *d, void *key, size_t len) {

    return art_delete((void *) d, key, len);
}

static inline void  *dict_find(struct Dict *d, void *key, size_t len) {

    return art_search((void *) d, key, len);
}


#undef EXPORT


#ifdef __cplusplus
}
#endif


#endif // #ifndef __DICT_H__

