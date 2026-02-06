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

#define NODE CAT(DNode, T)
#define LIST CAT(DList, T)

typedef struct NODE NODE;

struct NODE {
    T value;
    NODE* next;
    NODE* prev;
};


typedef struct LIST LIST;

struct LIST {
    NODE* head;
    NODE* tail;
    size_t len;
    AllocatorInstance allocator;
};

#define node_new CAT(dnode_new, T)
#define list_new CAT(dlist_new, T)
#define list_push_back CAT(dlist_push_back, T)
#define list_push_front CAT(dlist_push_front, T)
#define list_pop_front CAT(dlist_pop_front, T)
#define list_pop_back CAT(dlist_pop_back, T)
#define list_make_empty CAT(dlist_make_empty, T)

static NODE* node_new(AllocatorInstance allocator, const T value) {
    NODE* ptr = Allocator_alloc(allocator, TypeLayout_for_type(NODE));
    ptr->value = value;
    ptr->next = NULL;
    ptr->prev = NULL;
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
    self->len++;
    if (self->head == NULL) {
        self->head = node;
        self->tail = node;
        return;
    }
    self->tail->next = node;
    node->prev = self->tail;
    self->tail = node;
}

static void list_push_front(LIST* self, T value) {
    NODE* node = node_new(self->allocator, value);
    self->len++;
    if (self->head == NULL) {
        self->head = node;
        self->tail = node;
        return;
    }
    node->next = self->head;
    self->head->prev = node;
    self->head = node;
}

static bool list_pop_front(LIST* self, T* const out) {
    if (self->head == NULL) {
        return false;
    }
    self->len--;
    NODE* old_head = self->head;
    self->head = old_head->next;
    if (self->head == NULL) {
        self->tail = NULL;
    } else {
        self->head->prev = NULL;
    }
    *out = old_head->value;
    Allocator_free(self->allocator, TypeLayout_for_type(NODE), old_head);
    return true;
}

static bool list_pop_back(LIST* self, T* const out) {
    if (self->tail == NULL) {
        return false;
    }
    self->len--;
    NODE* old_tail = self->tail;
    self->tail = old_tail->prev;
    if (self->tail == NULL) {
        self->head = NULL;
    } else {
        self->tail->next = NULL;
    }
    *out = old_tail->value;
    Allocator_free(self->allocator, TypeLayout_for_type(NODE), old_tail);
    return true;
}

static void list_make_empty(LIST* list) {
    T sink;
    while (list_pop_front(list, &sink)) {
    }
}
#undef LIST
#undef NODE
#undef node_new
#undef list_new
#undef list_push_back
#undef list_push_front
#undef list_pop_front
#undef list_pop_back
#undef list_make_empty
#endif
