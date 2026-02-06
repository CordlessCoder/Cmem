// ReSharper disable once CppMissingIncludeGuard
#include <pthread.h>
#ifndef T
#error you need to define T before including this header
#else

#ifndef CAT
#define CAT_(A, B) A##_##B
#define CAT(A, B) CAT_(A, B)
#endif

#ifndef T_TL
#define T_TL(CAP) TypeLayout_new(CAP * sizeof(T), alignof(T))
#endif

#include "allocator.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef VEC_NO_BOUND_CHECKS
#include <stdio.h>
#endif

#define QUEUE CAT(Queue, T)

#define INTERNAL CAT(QueueInternal, T)

typedef struct QUEUE QUEUE;
typedef struct INTERNAL INTERNAL;

struct INTERNAL {
    pthread_mutex_t lock;
    pthread_cond_t empty_cv;
    pthread_cond_t full_cv;
};

struct QUEUE {
    T* ptr;
    size_t cap;
    size_t len;
    size_t start;
    INTERNAL* locking;
    AllocatorInstance allocator;
};

#define queue_new CAT(queue_new, T)
#define queue_push CAT(queue_push, T)
#define queue_pop CAT(queue_pop, T)
#define queue_free CAT(queue_free, T)

static QUEUE queue_new(AllocatorInstance allocator, size_t cap) {
    AALLOC_VAR(allocator, INTERNAL, internal);
    pthread_mutex_init(&internal->lock, NULL);
    pthread_cond_init(&internal->full_cv, NULL);
    pthread_cond_init(&internal->empty_cv, NULL);
    T* buf = Allocator_alloc(allocator, T_TL(cap));
    const QUEUE vec = {
        .ptr = buf,
        .cap = cap,
        .allocator = allocator,
        .locking = internal,
    };
    return vec;
}

static void queue_push(QUEUE* self, T value) {
    pthread_mutex_lock(&self->locking->lock);
    while (self->len == self->cap) {
        pthread_cond_wait(&self->locking->full_cv, &self->locking->lock);
    }
    // At this point we know there's space available in the queue
    size_t idx = (self->start + self->len++) % self->cap;
    self->ptr[idx] = value;
    pthread_cond_signal(&self->locking->empty_cv);
    pthread_mutex_unlock(&self->locking->lock);
}

static T queue_pop(QUEUE* self) {
    pthread_mutex_lock(&self->locking->lock);
    while (self->len == 0) {
        pthread_cond_wait(&self->locking->empty_cv, &self->locking->lock);
    }
    // At this point we know the queue isn't empty
    T ret = self->ptr[self->start];
    self->start = (self->start + 1) % self->cap;
    if (self->len == self->cap) {
        pthread_cond_signal(&self->locking->full_cv);
    }
    self->len--;
    pthread_mutex_unlock(&self->locking->lock);
    return ret;
}

static void queue_free(QUEUE* self) {
    if (self->ptr) {
        pthread_cond_destroy(&self->locking->empty_cv);
        pthread_cond_destroy(&self->locking->full_cv);
        pthread_mutex_destroy(&self->locking->lock);
        AFREE_VAR(self->allocator, self->locking);
        Allocator_free(self->allocator, T_TL(self->cap), self->ptr);
        self->cap = 0;
        self->len = 0;
        self->ptr = NULL;
    }
}


#undef LIST
#undef NODE
#undef T_TL
#undef T
#endif
