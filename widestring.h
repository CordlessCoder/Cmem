#ifndef WIDESTRING_H
#define WIDESTRING_H
#pragma once

#include "allocator.h"
#include <stddef.h>
#include <uchar.h>

typedef struct WideString WideString;
// A wide-pointer based UTF-8 string slice(reference).
struct WideString {
    char8_t* ptr;
    size_t len;
};

#define WS(str) (WideString){.ptr = (char8_t*)str, .len = strlen(str)}

typedef struct WideStringBuf WideStringBuf;
// A WideString that owns a buffer.
struct WideStringBuf {
    WideString str;
    size_t cap;
    AllocatorInstance allocator;
};

WideString WideString_new(char8_t* ptr, size_t len);

// Creates a new WideStringBuf
//
// NOTE: This function does not allocate.
WideStringBuf WideStringBuf_new(AllocatorInstance allocator);

// Ensures that at least `len` bytes can be written to the buffer without reallocating.
void WideStringBuf_reserve(WideStringBuf* self, size_t len);

// SAFETY: other must not point inside self.
void WideStringBuf_append(WideStringBuf* self, WideString other);

#endif
