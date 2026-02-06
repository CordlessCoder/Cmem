#include "list.h"
#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#include "allocator.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#define LIST CAT(LockedList, T)
#define INTERNAL CAT(LockedListInternal, T)

typedef struct INTERNAL INTERNAL;

struct INTERNAL {
    CAT(List, T) list;
    pthread_mutex_t lock;
};

typedef struct LIST LIST;

struct LIST {
    INTERNAL* inner;
};

#define llist_new CAT(locked_list_new, T)
#define llist_push_back CAT(locked_list_push_back, T)
#define llist_push_front CAT(locked_list_push_front, T)
#define llist_pop_front CAT(locked_list_pop_front, T)
#define llist_make_empty CAT(locked_list_make_empty, T)
#define llist_free CAT(locked_list_free, T)

static LIST llist_new(AllocatorInstance allocator) {
    CAT(List, T) list = CAT(list_new, T)(allocator);
    AALLOC_VAR(allocator, INTERNAL, inner);
    *inner = (INTERNAL){
        .list = list,
    };
    pthread_mutex_init(&inner->lock, NULL);
    return (LIST){
        .inner = inner,
    };
}

static void llist_push_back(LIST* self, const T value) {
    pthread_mutex_lock(&self->inner->lock);
    CAT(list_push_back, T)(&self->inner->list, value);
    pthread_mutex_unlock(&self->inner->lock);
}

static void llist_push_front(LIST* self, T value) {
    pthread_mutex_lock(&self->inner->lock);
    CAT(list_push_front, T)(&self->inner->list, value);
    pthread_mutex_unlock(&self->inner->lock);
}

static bool llist_pop_front(LIST* self, T* const out) {
    pthread_mutex_lock(&self->inner->lock);
    bool ret = CAT(list_pop_front, T)(&self->inner->list, out);
    pthread_mutex_unlock(&self->inner->lock);
    return ret;
}

static void llist_make_empty(LIST* self) {
    pthread_mutex_lock(&self->inner->lock);
    CAT(list_make_empty, T)(&self->inner->list);
    pthread_mutex_unlock(&self->inner->lock);
}

static void llist_free(LIST* self) {
    pthread_mutex_destroy(&self->inner->lock);
    AllocatorInstance alloc = self->inner->list.allocator;
    AFREE_VAR(alloc, self->inner);
}

#undef llist_new
#undef llist_push_back
#undef llist_push_front
#undef llist_pop_front
#undef llist_make_empty
#undef llist_free
#undef LIST
#undef NODE
#undef T
