#ifndef LOGGED_IN_LINKEDLIST_H
#define LOGGED_IN_LINKEDLIST_H

typedef struct Node {
    char *uid;
    struct Node *next;
} Node;

void addLoggedInUser(Node **head, const char *uid);

void removeLoggedInUser(Node **head, const char *uid);

int isUserLoggedIn(Node *head, const char *uid);

void freeList(Node *head);

#endif