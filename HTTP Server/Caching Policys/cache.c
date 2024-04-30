#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "cache.h"
#include <bits/getopt_core.h>

#define DEFAULT_CACHE_SIZE 1
#define BUFFER_SIZE        100

struct newItem {
    char item[50];
    int referenceBit;
};

struct cache {
    NewItem cache[BUFFER_SIZE];
    NewItem seenItems[BUFFER_SIZE];
    int totalSeen;
    int size;
    int capacity;
    int compulsoryMisses;
    int capacityMisses;
    char *policy;
};

Cache *cache_new(int capacity, char *policy) {
    Cache *cache = malloc(sizeof(Cache));
    cache->totalSeen = 0;
    cache->size = 0;
    cache->policy = policy;
    cache->capacity = capacity;
    cache->compulsoryMisses = 0;
    cache->capacityMisses = 0;
    return cache;
}

int getcompMisses(Cache *cache) {
    return cache->compulsoryMisses;
}

int getcapMisses(Cache *cache) {
    return cache->capacityMisses;
}

void searchHistory(Cache *cache, char *item) {
    int i;
    bool seen = 0;
    for (i = 0; i < cache->totalSeen; ++i) {
        //printf("%s", cache->seenItems[i].item);
        if (strcmp(cache->seenItems[i].item, item) == 0) {
            seen = 1;
            if (cache->size == cache->capacity) {
                //printf("Capacity Miss\n");
                cache->capacityMisses++;
            }
        }
    }
    //printf("I is: %i", i);
    if (i == cache->totalSeen && seen == 0) {
        //printf("Compulsory Miss\n");
        cache->compulsoryMisses++;
    }
}

void insert(Cache *cache, char *item) {
    // First check if item is in cache
    // HIT

    for (int i = 0; i < cache->size; ++i) {
        if (strcmp(cache->cache[i].item, item) == 0) {
            // HIT
            printf("HIT\n");
            // Can just return on a hit for FIFO

            // Move hit to the front for LRU
            if (strcmp(cache->policy, "-L") == 0) {
                NewItem temp = cache->cache[i];
                for (int j = i; j < cache->size - 1; j++) {
                    cache->cache[j] = cache->cache[j + 1];
                }
                cache->cache[cache->size - 1] = temp;
            }
            // Update clock bit to 1 for CLOCK
            if (strcmp(cache->policy, "-C") == 0) {
                cache->cache[i].referenceBit = 1;
            }

            return;
        }
    }
    searchHistory(cache, item);
    // MISS
    printf("MISS\n");
    int temp;

    // If not full, no eviction needed
    if (!(cache->size < cache->capacity)) {
        if (strcmp(cache->policy, "-F") == 0) {
            temp = 0;
        }

        if (strcmp(cache->policy, "-L") == 0) {
            temp = 0;
        }

        if (strcmp(cache->policy, "-C") == 0) {
            while (1) {
                if (cache->cache[cache->size - 1].referenceBit == 0) {
                    temp = cache->size - 1;
                    break;
                } else {
                    cache->cache[cache->size - 1].referenceBit = 0;
                    NewItem temp = cache->cache[cache->size - 1];
                    for (int j = cache->size - 1; j > 0; --j) {
                        cache->cache[j] = cache->cache[j - 1];
                    }
                    cache->cache[0] = temp;
                }
            }
        }

        // Dequeue the popped item
        //printf("Evicting: %s\n", cache->cache[index].item);
        cache->size--;
        for (int j = temp; j < cache->size; ++j) {
            cache->cache[j] = cache->cache[j + 1];
        }
    }

    // Add new item to location of evicted element
    // and update the cache history
    strcpy(cache->cache[cache->size].item, item);
    strcpy(cache->seenItems[cache->totalSeen].item, item);
    cache->cache[cache->size].referenceBit = 0;
    cache->size++;
    cache->totalSeen++;
}
