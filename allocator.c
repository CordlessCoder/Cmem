#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"

size_t next_pow2(size_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    if (sizeof(size_t) > 4) {
        v |= v >> 32;
    }
    v++;
    return v;
}

TypeLayout TypeLayout_new(size_t size, size_t align) {
    TypeLayout layout = {.size = size, .align = align};
    return layout;
}

AllocatorInstance AllocatorInstance_new(AllocatorVTable* vtable, void* allocator) {
    AllocatorInstance instance = {.vtable = vtable, .allocator = allocator};
    return instance;
}

void* Allocator_alloc(AllocatorInstance instance, TypeLayout layout) { return instance.vtable->alloc(instance.allocator, layout); }
void* Allocator_zeroed_alloc(AllocatorInstance instance, TypeLayout layout) { return instance.vtable->zeroed_alloc(instance.allocator, layout); }
void Allocator_free(AllocatorInstance instance, TypeLayout layout, void* ptr) { instance.vtable->free(instance.allocator, layout, ptr); }
void Allocator_destroy(AllocatorInstance instance) { instance.vtable->destroy_self(instance.allocator); }
