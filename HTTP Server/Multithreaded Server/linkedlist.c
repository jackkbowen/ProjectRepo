#include <stdio.h>
#include <stdlib.h>

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"
#include "rwlock.h"
#include "linkedlist.h"

struct node {
    char *URI;
    rwlock_t *rwlock;
    struct node *next;
};

struct list {
    Node *head;
};

Node *createnode(char *URI, rwlock_t *rwlock);

Node *createnode(char *URI, rwlock_t *rwlock) {
    Node *newNode = malloc(sizeof(Node));
    if (!newNode) {
        return NULL;
    }
    newNode->URI = URI;
    newNode->rwlock = rwlock;
    newNode->next = NULL;

    return newNode;
}

List *makelist() {
    List *list = malloc(sizeof(List));
    if (!list) {
        return NULL;
    }
    list->head = NULL;
    return list;
}

bool compareURI(List *list, char *URI) {
    Node *current = list->head;
    if (list->head == NULL)
        return 0;

    for (; current != NULL; current = current->next) {
        printf("%s\n", current->URI);
        if (current->URI == URI) {
            return 1;
        }
    }
    return 0;
}

void add(char *URI, rwlock_t *rwlock, List *list) {
    Node *current = NULL;
    if (list->head == NULL) {
        list->head = createnode(URI, rwlock);
    } else {
        current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = createnode(URI, rwlock);
    }
}

void delete (char *URI, List *list) {
    Node *current = list->head;
    Node *previous = current;
    while (current != NULL) {
        if (current->URI == URI) {
            previous->next = current->next;
            if (current == list->head)
                list->head = current->next;
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void destroy(List *list) {
    Node *current = list->head;
    Node *next = current;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
