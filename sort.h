// ReSharper disable once CppMissingIncludeGuard
#ifndef T
#error you need to define T before including this header
#else


#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#include <stdbool.h>
#include <stdlib.h>

#define CMP_FN CAT(compare, T)
#define SORT_FN CAT(quicksort, T)
#define PARTITION_FN CAT(partition, T)

size_t PARTITION_FN(T* ptr, size_t len) {
    {
        T tmp = ptr[len / 2];
        ptr[len / 2] = ptr[len - 1];
        ptr[len - 1] = tmp;
    }
    T pivot = ptr[len - 1];

    T* end = ptr + len - 1;
    T* write = ptr;
    T* read = ptr;

    while (read < end) {
        if (CMP_FN(*read, pivot) != 1) {
            T tmp = *write;
            *write = *read;
            *read = tmp;
            write++;
        }
        read++;
    }
    T tmp = *write;
    *write = *end;
    *end = tmp;
    return write - ptr;
}

void SORT_FN(T* ptr, size_t len) {
    int (*_cmp_fn)(T a, T b);
    1 ? CMP_FN : _cmp_fn;
    if (len < 2) {
        return;
    }

    size_t partition_point = PARTITION_FN(ptr, len);
    SORT_FN(ptr, partition_point);
    SORT_FN(ptr + partition_point + 1, len - partition_point - 1);
}

#undef SORT_FN
#undef PARTITION_FN
#undef T
#endif
