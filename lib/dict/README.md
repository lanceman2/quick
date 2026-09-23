# A TRIE based C Dictionary library API

An adaptive [TRIE](https://en.wikipedia.org/wiki/Trie) dictionary data
structure C library API.

Rather than write write our own we wrap an existing API which
we choice based on a benchmark we did at
[dictionary_benchmark/](https://github.com/lanceman2/dictionary_benchmark/).

We wrap the C API with our own API.  We could change the under laying
library API without effecting the wrapper API that we build on.
The wrapper API uses C static inline functions from
[../../include/dict.h](../../include/dict.h).

