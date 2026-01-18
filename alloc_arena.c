#include "allocator.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PAGE_ALIGN 4096

typedef struct ArenaPageHeader {
    struct ArenaPageHeader* prev;
    size_t capacity, used;
} ArenaPageHeader;


typedef struct ArenaAlloc {
    ArenaPageHeader* page;
    AllocatorInstance allocate_in;
    size_t page_capacity;
    size_t max_arena_alloc;
} ArenaAlloc;

uintptr_t round_up_to_multiple(uintptr_t ptr, size_t factor) {
    // Ceil division followed my multiplication
    return ((ptr + (factor - 1)) / factor) * factor;
}


ArenaPageHeader* ArenaPage_alloc(AllocatorInstance allocate_in, ArenaPageHeader* prev, size_t capacity) {
    assert(capacity >= sizeof(ArenaPageHeader));
    capacity = round_up_to_multiple(capacity, PAGE_ALIGN);
    ArenaPageHeader* new = Allocator_alloc(allocate_in, TypeLayout_new(capacity, PAGE_ALIGN));
    assert(new != NULL);
    new->prev = prev;
    new->capacity = capacity;
    new->used = sizeof(ArenaPageHeader);
    return new;
}

void* ArenaPage_current_ptr(ArenaPageHeader* page) {
    char* ptr = (char*)page + page->used;
    return ptr;
}

void* ArenaPage_end_ptr(ArenaPageHeader* page) {
    char* ptr = (char*)page + page->capacity;
    return ptr;
}

// Returns a non-null value if the allocation succeeded
// Returns null if an allocation could not be performed as the page does not have sufficient capacity available
void* ArenaPage_try_alloc(ArenaPageHeader* page, TypeLayout layout) {
    if (!page) {
        // Cannot allocate in null page
        return NULL;
    }
    void* current_ptr = ArenaPage_current_ptr(page);
    uintptr_t padded_to_alignment = round_up_to_multiple((uintptr_t)current_ptr, layout.align);
    void* end_of_value = (void*)(padded_to_alignment + layout.size);
    if (end_of_value > ArenaPage_end_ptr(page)) {
        // Value did not fit in page
        return NULL;
    }
    page->used = (uintptr_t)end_of_value - (uintptr_t)page;
    return (void*)padded_to_alignment;
}

void ArenaPage_free(AllocatorInstance free_in, ArenaPageHeader* page) {
    while (page) {
        ArenaPageHeader* prev = page->prev;
        size_t size = page->capacity;
        // Free buffer
        Allocator_free(free_in, TypeLayout_new(size, PAGE_ALIGN), page);
        page = prev;
    }
}

void arena_destroy_self(void* self) {
    if (!self) {
        // Guard against null self
        return;
    }
    ArenaAlloc* arena = self;
    AllocatorInstance allocate_in = arena->allocate_in;
    ArenaPage_free(allocate_in, arena->page);
    arena->page = NULL;
    AFREE(allocate_in, ArenaAlloc, arena);
}
void* arena_alloc(void* self, TypeLayout layout) {
    // printf("Allocating %lu bytes with an alignment of %lu\n", layout.size, layout.align);
    if (!self) {
        // Guard against null self
        return NULL;
    }
    ArenaAlloc* arena = self;
    void* alloc = ArenaPage_try_alloc(arena->page, layout);
    if (alloc) {
        // Allocated in the existing page
        return alloc;
    }
    if (layout.size > arena->max_arena_alloc) {
        // Need to allocate a new page, just for this allocation.
        ArenaPageHeader* page = arena->page;
        ArenaPageHeader* old_prev = NULL;
        if (page) {
            old_prev = page->prev;
            page->prev = NULL;
        }
        // Reserve enough space for an ArenaPageHeader
        size_t min_cap = round_up_to_multiple(sizeof(ArenaPageHeader) + layout.size, layout.align);
        ArenaPageHeader* new_prev = ArenaPage_alloc(arena->allocate_in, old_prev, min_cap);
        void* ptr = ArenaPage_try_alloc(new_prev, layout);
        assert(ptr != NULL);
        if (page) {
            page->prev = new_prev;
        } else {
            arena->page = new_prev;
        }
        return ptr;
    }
    // Need to allocate a new page
    arena->page = ArenaPage_alloc(arena->allocate_in, arena->page, arena->page_capacity);
    alloc = ArenaPage_try_alloc(arena->page, layout);
    return alloc;
}
void* arena_zeroed_alloc(void* self, TypeLayout layout) {
    void* ptr = arena_alloc(self, layout);
    memset(ptr, 0, layout.size);
    return ptr;
}
void arena_free(void* self, TypeLayout layout, void* ptr) {}

static AllocatorVTable AllocatorInstance_ArenaVTable = {.alloc = arena_alloc, .zeroed_alloc = arena_zeroed_alloc, .free = arena_free, .destroy_self = arena_destroy_self};

AllocatorInstance AllocatorInstance_arena(AllocatorInstance allocate_in) {
    ArenaAlloc* allocator = AALLOC(allocate_in, ArenaAlloc);
    allocator->page_capacity = 1024 * 64;
    allocator->allocate_in = allocate_in;
    allocator->max_arena_alloc = 1024 * 8;
    allocator->page = NULL;
    AllocatorInstance instance = {
        .vtable = &AllocatorInstance_ArenaVTable,
        .allocator = allocator,
    };
    return instance;
}
