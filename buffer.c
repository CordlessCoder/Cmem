#include "buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ByteSlice ByteSlice_fromptr(char* ptr, size_t length) {
    ByteSlice self = {.ptr = ptr, .length = length};
    return self;
}

char* ByteSlice_get(ByteSlice* self, size_t idx) {
    if (idx >= self->length) {
        fprintf(stderr, "Out of bounds access on byte slice: index %zu is not in bounds for a slice of length %zu", idx, self->length);
        exit(127);
    }
    return &self->ptr[idx];
}

unsigned long ByteSlice_write(ByteSlice self, FILE* file) { return fwrite(self.ptr, 1, self.length, file); }

ByteBuffer ByteBuffer_alloc(AllocatorInstance allocator, size_t capacity) {
    char* ptr = Allocator_alloc(allocator, TypeLayout_new(capacity, 1));
    ByteBuffer buf = {
        .ptr = ptr,
        .capacity = capacity,
        .length = 0,
    };
    return buf;
}

void ByteBuffer_free(AllocatorInstance allocator, ByteBuffer self) { Allocator_free(allocator, TypeLayout_new(self.capacity, 1), self.ptr); }

ByteSlice ByteBuffer_as_slice(ByteBuffer* self) { return ByteSlice_fromptr(self->ptr, self->length); }

void ByteBuffer_reserve(AllocatorInstance allocator, ByteBuffer* self, size_t extra) {
    size_t needed = self->length + extra;
    if (needed <= self->capacity) {
        return;
    }
    ByteBuffer new = ByteBuffer_alloc(allocator, next_pow2(needed));
    memcpy(new.ptr, self->ptr, self->length);
    new.length = self->length;
    ByteBuffer_free(allocator, *self);
    *self = new;
}

void ByteBuffer_append(AllocatorInstance allocator, ByteBuffer* self, ByteSlice slice) {
    ByteBuffer_reserve(allocator, self, slice.length);
    memcpy(&self->ptr[self->length], slice.ptr, slice.length);
    self->length += slice.length;
}

void ByteBuffer_push(AllocatorInstance allocator, ByteBuffer* self, char byte) { ByteBuffer_append(allocator, self, ByteSlice_fromptr(&byte, 1)); }
bool ByteBuffer_pop(ByteBuffer* self, char* byte) {
    if (self->length == 0) {
        return false;
    }
    self->length--;
    *byte = self->ptr[self->length];
    return true;
}
