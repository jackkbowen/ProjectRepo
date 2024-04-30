// Credit: https://github.com/skorks/c-linked-list/blob/master/linkedlist.h

#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

typedef struct node Node;

typedef struct list List;

List *makelist();
void add(char *URI, rwlock_t *rwlock, List *list);
void delete (char *URI, List *list);
bool compareURI(List *list, char *URI);
void destroy(List *list);

#endif
