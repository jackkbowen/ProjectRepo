#ifndef CACHE_HEADER
#define CACHE_HEADER

typedef struct newItem NewItem;
typedef struct cache Cache;

Cache *cache_new(int capacity, char *policy);
int getcompMisses(Cache *cache);
int getcapMisses(Cache *cache);
void searchHistory(Cache *cache, char *item);
void insert(Cache *cache, char *item);

#endif
