#include "allocator.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct ByteBuffer {
    char* ptr;
    size_t capacity;
    size_t length;
} ByteBuffer;

typedef struct ByteSlice {
    char* ptr;
    size_t length;
} ByteSlice;

ByteSlice ByteSlice_fromptr(char* ptr, size_t length);
char* ByteSlice_get(ByteSlice* self, size_t idx);
unsigned long ByteSlice_write(ByteSlice self, FILE* file);

ByteBuffer ByteBuffer_alloc(AllocatorInstance allocator, size_t capacity);
void ByteBuffer_free(AllocatorInstance allocator, ByteBuffer self);
ByteSlice ByteBuffer_as_slice(ByteBuffer* self);
void ByteBuffer_reserve(AllocatorInstance allocator, ByteBuffer* self, size_t extra);
void ByteBuffer_push(AllocatorInstance allocator, ByteBuffer* self, char byte);
void ByteBuffer_append(AllocatorInstance allocator, ByteBuffer* self, ByteSlice slice);
bool ByteBuffer_pop(ByteBuffer* self, char* byte);
