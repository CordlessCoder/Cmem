// ReSharper disable once CppMissingIncludeGuard
#ifndef T
#error you need to define T before including this header
#else

#ifndef NATURAL_CMP
#define NATURAL_CMP(T)                                                                                                                                                                                 \
    int CAT(natural_cmp, T)(T a, T b) {                                                                                                                                                                \
        if (a > b) {                                                                                                                                                                                   \
            return 1;                                                                                                                                                                                  \
        }                                                                                                                                                                                              \
        if (a < b) {                                                                                                                                                                                   \
            return -1;                                                                                                                                                                                 \
        }                                                                                                                                                                                              \
        return 0;                                                                                                                                                                                      \
    }
#endif

#undef T
#endif
