#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#pragma once
#include <stdalign.h>
#include <stddef.h>

typedef struct TypeLayout {
    size_t size, align;
} TypeLayout;

typedef struct AllocatorVTable {
    void* (*alloc)(void* self, TypeLayout layout);
    void* (*zeroed_alloc)(void* self, TypeLayout layout);
    void (*free)(void* self, TypeLayout layout, void* ptr);
    void (*destroy_self)(void* self);
} AllocatorVTable;

typedef struct AllocatorInstance {
    AllocatorVTable* vtable;
    void* allocator;
} AllocatorInstance;

TypeLayout TypeLayout_new(size_t size, size_t align);
AllocatorInstance AllocatorInstance_new(AllocatorVTable* vtable, void* allocator);

void* Allocator_alloc(AllocatorInstance instance, TypeLayout layout);
void* Allocator_zeroed_alloc(AllocatorInstance instance, TypeLayout layout);
void Allocator_free(AllocatorInstance instance, TypeLayout layout, void* ptr);
void Allocator_destroy(AllocatorInstance instance);

AllocatorInstance AllocatorInstance_malloc_wrapper(void);
AllocatorInstance AllocatorInstance_arena(AllocatorInstance allocate_in);

size_t next_pow2(size_t v);

// clang-format off
#define AALLOC(INSTANCE, TYPE) ((TYPE*)Allocator_alloc(INSTANCE, TypeLayout_for_type(TYPE)))
#define AALLOC_VAR(INSTANCE, TYPE, NAME) TYPE* NAME = ((TYPE*)Allocator_alloc(INSTANCE, TypeLayout_for_type(TYPE)))
#define ACALLOC(INSTANCE, TYPE) ((TYPE*)Allocator_zeroed_alloc(INSTANCE, TypeLayout_for_type(TYPE)))
#define ACALLOC_VAR(INSTANCE, TYPE, NAME) TYPE* NAME = ((TYPE*)Allocator_zeroed_alloc(INSTANCE, TypeLayout_for_type(TYPE)))
#define AFREE(INSTANCE, TYPE, PTR) (Allocator_free(INSTANCE, TypeLayout_for_type(TYPE), PTR))
#define AFREE_VAR(INSTANCE, VAR) (Allocator_free(INSTANCE, TypeLayout_for_type(*VAR), VAR))
#define TypeLayout_for_type(TYPE) ((TypeLayout)\
    { .size = sizeof(TYPE), .align = alignof(TYPE) })
#define TypeLayout_array(TYPE, COUNT) ((TypeLayout)\
    { .size = sizeof(TYPE) * COUNT, .align = alignof(TYPE) })
// clang-format on
#endif
