// ReSharper disable once CppMissingIncludeGuard
#ifndef K
#error you need to define K before including this header
#elifndef V
#error you need to define V before including this header
#else

#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#include "allocator.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define KV CAT(K, V)

#define TABLE CAT(Table, KV)
#define NODE CAT(TableNode, KV)
#define ITER CAT(TableIter, KV)

#ifndef BUF_TL
#define BUF_TL(CAP) TypeLayout_new(CAP * sizeof(NODE), alignof(NODE))
#endif

#ifndef LOAD_FACTOR
#define LOAD_FACTOR(CAP) CAP * 3 / 2
#endif

typedef struct NODE NODE;

struct NODE {
    bool present;
    K key;
    V value;
};

typedef struct TABLE TABLE;

struct TABLE {
    NODE* ptr;
    size_t cap;
    size_t len;
    AllocatorInstance allocator;
};

typedef struct ITER ITER;

struct ITER {
    NODE* ptr;
    size_t cap;
    size_t idx;
};

#define table_new CAT(table_new, KV)
#define table_reserve CAT(table_reserve, KV)
#define table_insert CAT(table_insert, KV)
#define table_get CAT(table_get, KV)
#define table_remove CAT(table_remove, KV)
#define table_copy CAT(table_copy, KV)
#define table_iter CAT(table_iter, KV)
#define table_iter_next CAT(table_iter_next, KV)
#define table_make_empty CAT(table_make_empty, KV)
#define table_compute_buffer_size CAT(table_compute_buffer_size, KV)
#define table_free CAT(table_free, KV)

static TABLE table_new(AllocatorInstance allocator) {
    return (TABLE){
        .ptr = NULL,
        .cap = 0,
        .len = 0,
        .allocator = allocator,
    };
}

static void table_free(TABLE* self) {
    if (self->cap) {
        Allocator_free(self->allocator, BUF_TL(self->cap), self->ptr);
    }
    self->cap = 0;
    self->len = 0;
    self->ptr = NULL;
}

static void table_reserve(TABLE* self, size_t extra);

// Using c_1 = c_2 = 0.5
#ifndef QUADRATIC_PROBE
#define QUADRATIC_PROBE(HASH, I, SIZE) (HASH + (I + I * I) / 2) % SIZE
#endif

static void table_insert(TABLE* self, K key, V value) {
    table_reserve(self, 1);
    uint64_t (*hash_fn)(K) = CAT(table_hash, K);
    bool (*eq_fn)(K, K) = CAT(table_eq, K);
    uint64_t hash = hash_fn(key);
    for (size_t probe = 0; probe < self->cap; probe++) {
        size_t idx = QUADRATIC_PROBE(hash, probe, self->cap);
        bool present = self->ptr[idx].present;
        if (present && !eq_fn(self->ptr[idx].key, key)) {
            // Collision
            continue;
        }
        self->ptr[idx].key = key;
        self->ptr[idx].value = value;
        self->ptr[idx].present = true;
        if (!present) {
            self->len++;
        }
        return;
    }
    // NOTE: Should be impossible if quadratic probing is fully cyclical, which should be true for the constants used
    __builtin_unreachable();
}

static V* table_get(const TABLE* self, K key) {
    uint64_t (*hash_fn)(K) = CAT(table_hash, K);
    bool (*eq_fn)(K, K) = CAT(table_eq, K);
    uint64_t hash = hash_fn(key);
    for (size_t probe = 0; probe < self->cap; probe++) {
        size_t idx = QUADRATIC_PROBE(hash, probe, self->cap);
        if (!self->ptr[idx].present) {
            // Not present
            return NULL;
        }
        if (eq_fn(key, self->ptr[idx].key)) {
            return &self->ptr[idx].value;
        }
    }
    return NULL;
}

static bool table_remove(TABLE* self, K key, V* out) {
    uint64_t (*hash_fn)(K) = CAT(table_hash, K);
    bool (*eq_fn)(K, K) = CAT(table_eq, K);
    uint64_t hash = hash_fn(key);
    NODE* to_be_removed = NULL;
    NODE* end_of_probe_chain = NULL;
    size_t probe = 0;
    for (size_t probe = 0; probe < self->cap; probe++) {
        size_t idx = QUADRATIC_PROBE(hash, probe, self->cap);
        NODE* probed_node = &self->ptr[idx];
        if (!probed_node->present) {
            // End of probe chain
            break;
        }
        end_of_probe_chain = probed_node;
        if (eq_fn(key, probed_node->key)) {
            to_be_removed = probed_node;
        }
    }
    if (to_be_removed == NULL) {
        return false;
    }
    *out = to_be_removed->value;
    // Swap end of probe chain in place of the removed node
    if (to_be_removed != end_of_probe_chain) {
        to_be_removed->value = end_of_probe_chain->value;
        to_be_removed->key = end_of_probe_chain->key;
    }
    end_of_probe_chain->present = false;
    self->len--;
    return true;
}

static bool table_iter_next(ITER* iter, NODE** out);
static ITER table_iter(TABLE* self);
static void table_make_empty(TABLE* self);

static void table_reserve(TABLE* self, size_t extra) {
    size_t new_cap = next_pow2(LOAD_FACTOR(self->len + extra));
    if (new_cap == 0) {
        return;
    }
    if (self->cap == 0) {
        self->ptr = Allocator_alloc(self->allocator, BUF_TL(new_cap));
        self->cap = new_cap;
        self->len = 0;
        table_make_empty(self);
        return;
    }
    if (new_cap <= self->cap) {
        return;
    }
    TABLE new_table = table_new(self->allocator);
    table_reserve(&new_table, new_cap);

    // Copy old elements over to new table
    ITER iter = table_iter(self);
    NODE* out;
    while (table_iter_next(&iter, &out)) {
        table_insert(&new_table, out->key, out->value);
    }

    table_free(self);
    *self = new_table;
}

static ITER table_iter(TABLE* self) { return (ITER){.ptr = self->ptr, .cap = self->cap, .idx = 0}; }

static bool table_iter_next(ITER* iter, NODE** out) {
    while (iter->idx < iter->cap) {
        if (iter->ptr[iter->idx].present) {
            *out = &iter->ptr[iter->idx++];
            return true;
        }
        iter->idx++;
    }
    return false;
}

static TABLE table_copy(TABLE* self) {
    TABLE new_table = table_new(self->allocator);

    // Copy old elements over to new table
    ITER iter = table_iter(self);
    NODE* out;
    while (table_iter_next(&iter, &out)) {
        table_insert(&new_table, out->key, out->value);
    }

    return new_table;
}

static void table_make_empty(TABLE* self) {
    for (size_t i = 0; i < self->cap; i++) {
        self->ptr[i].present = false;
    }
    self->len = 0;
}
static size_t table_compute_buffer_size(const TABLE* self) { return self->cap * sizeof(NODE); }

#undef LIST
#undef NODE
#undef ITER
#undef BUF_TL
#undef K
#undef V
#endif
