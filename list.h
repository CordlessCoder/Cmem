// ReSharper disable once CppMissingIncludeGuard
#ifndef T
#error you need to define T before including this header
#else

#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#include "allocator.h"
#include <stdbool.h>
#include <stdlib.h>

#define NODE CAT(Node, T)
#define LIST CAT(List, T)

typedef struct NODE NODE;

struct NODE {
    T value;
    NODE* next;
};


typedef struct LIST LIST;

struct LIST {
    NODE* head;
    NODE* tail;
    AllocatorInstance allocator;
};

#define node_new CAT(node_new, T)
#define list_new CAT(list_new, T)
#define list_push_back CAT(list_push_back, T)
#define list_push_front CAT(list_push_front, T)
#define list_pop_front CAT(list_pop_front, T)
#define list_make_empty CAT(list_make_empty, T)

static NODE* node_new(AllocatorInstance allocator, const T value) {
    NODE* ptr = Allocator_alloc(allocator, TypeLayout_for_type(NODE));
    ptr->value = value;
    ptr->next = NULL;
    return ptr;
}

static LIST list_new(AllocatorInstance allocator) {
    const LIST list = {
        .head = NULL,
        .tail = NULL,
        .allocator = allocator,
    };
    return list;
}

static void list_push_back(LIST* self, const T value) {
    NODE* node = node_new(self->allocator, value);
    if (self->head == NULL) {
        self->head = node;
        self->tail = node;
        return;
    }
    self->tail->next = node;
    self->tail = node;
}

static void list_push_front(LIST* self, T value) {
    NODE* node = node_new(self->allocator, value);
    if (self->head == NULL) {
        self->head = node;
        self->tail = node;
        return;
    }
    node->next = self->head;
    self->head = node;
}

static bool list_pop_front(LIST* self, T* const out) {
    if (self->head == NULL) {
        return false;
    }
    NODE* old_head = self->head;
    self->head = old_head->next;
    if (self->head == NULL) {
        self->tail = NULL;
    }
    *out = old_head->value;
    Allocator_free(self->allocator, TypeLayout_for_type(NODE), old_head);
    return true;
}

static void list_make_empty(LIST* list) {
    T sink;
    while (list_pop_front(list, &sink)) {
    }
}
#undef LIST
#undef NODE
#undef T
#endif
