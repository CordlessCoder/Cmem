#include "widestring.h"
#include "allocator.h"

WideString WideString_new(char8_t* ptr, size_t len) { return (WideString){.ptr = ptr, .len = len}; }

WideStringBuf WideStringBuf_new(AllocatorInstance allocator) {
    return (WideStringBuf){
        .str = WideString_new(NULL, 0),
        .allocator = allocator,
        .cap = 0,
    };
}

void WideStringBuf_reserve(WideStringBuf* self, size_t len) {
    size_t needed = self->str.len + len;
    if (self->cap >= needed) {
        return;
    }
    size_t new_cap = next_pow2(needed);
}

// SAFETY: other must not point inside self.
// TODO
void WideStringBuf_append(WideStringBuf* self, WideString other) {
}
