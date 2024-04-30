#include "rwlock.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
pthread_mutex_t mutex;
pthread_cond_t reader;
pthread_cond_t writer;

struct rwlock {
    int numReaders;
    int numWriters;
    int waitingReaders;
    int waitingWriters;
    int prioValue;
    int prioVar;
};

typedef struct rwlock rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rwlocker = (rwlock_t *) malloc(sizeof(rwlock_t));
    rwlocker->numReaders = 0;
    rwlocker->numWriters = 0;
    rwlocker->waitingReaders = 0;
    rwlocker->waitingWriters = 0;
    rwlocker->prioValue = n;
    rwlocker->prioVar = p;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&reader, NULL);
    pthread_cond_init(&writer, NULL);

    return rwlocker;
}

void rwlock_delete(rwlock_t **rw) {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&reader);
    pthread_cond_destroy(&writer);
    free(*rw);
    *rw = NULL;
}

void reader_lock(rwlock_t *rw) {
    printf("Before reader_Lock locks mutex\n");
    pthread_mutex_lock(&mutex);
    printf("Reader got lock\n");
    rw->waitingReaders = rw->waitingReaders + 1;

    if (rw->waitingWriters == 0 && rw->numWriters == 0) {
        printf("No Contention, adding reader\n");
        rw->waitingReaders = rw->waitingReaders - 1;
        rw->numReaders = rw->numReaders + 1;
        pthread_mutex_unlock(&mutex);
        return;
    }

    switch (rw->prioVar) {
    // READERS CASE
    case 0:
        while (rw->numWriters > 0) {
            printf("active writers, reader_lock waiting\n");
            pthread_cond_wait(&reader, &mutex);
        }
        rw->waitingReaders = rw->waitingReaders - 1;
        rw->numReaders = rw->numReaders + 1;
        printf("Before reader_lock unlocks mutex\n");
        break;

    // WRITERS CASE
    case 1:
        while (rw->numWriters > 0 || rw->waitingWriters > 0) {
            printf("active writers, reader_lock waiting\n");
            pthread_cond_wait(&reader, &mutex);
        }
        rw->waitingReaders = rw->waitingReaders - 1;
        rw->numReaders = rw->numReaders + 1;
        printf("Before reader_lock unlocks mutex\n");
        break;

    // N_WAY CASE
    case 2: printf("In N_WAY reader_lock\n"); break;
    }

    pthread_mutex_unlock(&mutex);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&mutex);
    rw->numReaders = rw->numReaders - 1;
    printf("\nnumReaders: %d\n", rw->numReaders);
    printf("waitingWriters: %d\n\n", rw->waitingWriters);
    switch (rw->prioVar) {
    case 0:
        if (rw->numReaders == 0 && rw->waitingWriters > 0) {
            printf("in reader unlock case before signal\n");
            pthread_cond_signal(&writer);
        }
        break;
    case 1:
        if (rw->numReaders == 0 && rw->waitingWriters > 0) {
            pthread_cond_signal(&writer);
        }
        break;
    case 2: printf("In N_WAY reader_unlock\n"); break;
    }
    printf("Unlocking Mutex\n");
    pthread_mutex_unlock(&mutex);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&mutex);
    printf("Writer got lock\n");
    rw->waitingWriters = rw->waitingWriters + 1;

    if (rw->numReaders == 0 && rw->numWriters == 0) {
        printf("No Contention, adding writer\n");
        rw->waitingWriters = rw->waitingWriters - 1;
        rw->numWriters = rw->numWriters + 1;
        pthread_mutex_unlock(&mutex);
        return;
    }

    switch (rw->prioVar) {
    // READERS CASE
    case 0:
        while (rw->numReaders > 0 || rw->numWriters > 0) {
            printf("active readers, writer_lock waiting\n");
            pthread_cond_wait(&writer, &mutex);
        }
        rw->waitingWriters = rw->waitingWriters - 1;
        rw->numWriters = rw->numWriters + 1;
        printf("Before writer_lock unlocks mutex\n");
        break;

    // WRITERS CASE
    case 1:
        while (rw->numReaders > 0 || rw->numWriters > 0) {
            printf("active writers, reader_lock waiting\n");
            pthread_cond_wait(&writer, &mutex);
        }
        rw->waitingReaders = rw->waitingReaders - 1;
        rw->numReaders = rw->numReaders + 1;
        printf("Before reader_lock unlocks mutex\n");
        break;

    // N_WAY CASE
    case 2: printf("In N_WAY writer lock\n"); break;
    }
    pthread_mutex_unlock(&mutex);
}

void writer_unlock(rwlock_t *rw) {
    printf("Num Writers: %d\n", rw->numWriters);
    pthread_mutex_lock(&mutex);
    rw->numWriters = 0;
    //assert(rw->numWriters == 0);
    printf("Num Writers: %d\n", rw->numWriters);
    if (rw->waitingWriters > 0) {
        pthread_cond_signal(&writer);
    } else
        pthread_cond_broadcast(&reader);

    pthread_mutex_unlock(&mutex);
}
