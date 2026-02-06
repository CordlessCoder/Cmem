// ReSharper disable once CppMissingIncludeGuard
#ifndef T
#error you need to define T before including this header
#else

#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#ifndef T_TL
#define T_TL(CAP) TypeLayout_new(CAP * sizeof(T), alignof(T))
#endif

#include "allocator.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef VEC_NO_BOUND_CHECKS
#include <stdio.h>
#endif

#define VECTOR CAT(Vector, T)

typedef struct VECTOR VECTOR;

struct VECTOR {
    T* ptr;
    size_t cap;
    size_t len;
    AllocatorInstance allocator;
};

#define vector_new CAT(vector_new, T)
#define vector_reserve CAT(vector_reserve, T)
#define vector_get CAT(vector_get, T)
#define vector_push_back CAT(vector_push_back, T)
#define vector_append_back CAT(vector_append_back, T)
#define vector_insert CAT(vector_insert, T)
#define vector_pop_back CAT(vector_pop_back, T)
#define vector_remove CAT(vector_remove, T)
#define vector_make_empty CAT(vector_make_empty, T)
#define vector_free CAT(vector_free, T)

static VECTOR vector_new(AllocatorInstance allocator) {
    const VECTOR vec = {
        .ptr = NULL,
        .cap = 0,
        .len = 0,
        .allocator = allocator,
    };
    return vec;
}

static void vector_free(VECTOR* self) {
    if (self->cap) {
        Allocator_free(self->allocator, T_TL(self->cap), self->ptr);
    }
    self->cap = 0;
    self->len = 0;
    self->ptr = NULL;
}

static void vector_reserve(VECTOR* self, const size_t extra) {
    if (!self->cap) {
        // Vector hasn't been allocated yet
        size_t new_cap = next_pow2(extra);
        self->ptr = Allocator_alloc(self->allocator, T_TL(new_cap));
        self->cap = new_cap;
        return;
    }
    if (self->len + extra <= self->cap) {
        // We have sufficient capacity
        return;
    }
    size_t new_cap = next_pow2(self->len + extra);
    // Allocate buffer buffer
    T* new_buf = Allocator_alloc(self->allocator, T_TL(new_cap));
    // Copy old elements over
    memcpy(new_buf, self->ptr, sizeof(T) * self->len);
    size_t new_len = self->len;
    vector_free(self);
    self->len = new_len;
    self->ptr = new_buf;
    self->cap = new_cap;
}

static T* vector_get(VECTOR* self, const size_t idx) {
#ifndef VEC_NO_BOUND_CHECKS
    if (idx >= self->len) {
        fprintf(stderr, ("Index %zu is out of bounds for vector of length %zu"), idx, self->len);
        exit(127);
    }
#endif
    return &self->ptr[idx];
}

static void vector_push_back(VECTOR* self, const T value) {
    vector_reserve(self, 1);
    self->ptr[self->len++] = value;
}

static void vector_append_back(VECTOR* self, const T* ptr, size_t len) {
    vector_reserve(self, len);
    memcpy(&self->ptr[self->len], ptr, len * sizeof(T));
    self->len += len;
}

static bool vector_pop_back(VECTOR* self, T* const out) {
    if (self->len == 0) {
        return false;
    }
    *out = self->ptr[--self->len];
    return true;
}

static bool vector_remove(VECTOR* self, size_t idx, T* const out) {
    if (idx >= self->len) {
        return false;
    }
    *out = self->ptr[idx];
    memmove(&self->ptr[idx], &self->ptr[idx + 1], (self->len - idx - 1) * sizeof(T));
    self->len--;
    return true;
}

static void vector_make_empty(VECTOR* self) { self->len = 0; }

static void vector_insert(VECTOR* self, const size_t idx, const T value) {
    vector_reserve(self, 1);
    memmove(self->ptr + idx + 1, self->ptr, (self->len - idx) * sizeof(T));
    self->ptr[idx] = value;
    self->len++;
}

#undef LIST
#undef NODE
#undef T_TL
#undef T
#endif
