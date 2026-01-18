#include "allocator.h"
#include <string.h>
#include <stdlib.h>

void malloc_destroy_self(void* self) {}
void* malloc_alloc(void* self, TypeLayout layout) { return aligned_alloc(layout.align, layout.size); }
void* malloc_zeroed_alloc(void* self, TypeLayout layout) { 
    void* ptr = malloc_alloc(self, layout);
    memset(ptr, 0, layout.size);
    return ptr;
}
void malloc_free(void* self, TypeLayout layout, void* ptr) {
    (void)(self);
    (void)(layout);
    free(ptr);
}

static AllocatorVTable AllocatorInstance_MallocVTable = {.alloc = malloc_alloc, .zeroed_alloc = malloc_zeroed_alloc, .free = malloc_free, .destroy_self = malloc_destroy_self};
static AllocatorInstance AllocatorInstance_Malloc = {.vtable = &AllocatorInstance_MallocVTable, .allocator = NULL};

// Returns a handle to a global allocator wrapping malloc.
AllocatorInstance AllocatorInstance_malloc_wrapper(void) { return AllocatorInstance_Malloc; }
