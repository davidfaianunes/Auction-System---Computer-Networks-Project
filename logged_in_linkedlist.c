#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logged_in_linkedlist.h"


// Function to add a UID to the list of logged-in users
void addLoggedInUser(Node **head, const char *uid) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the UID and copy it
    newNode->uid = strdup(uid);
    if (newNode->uid == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->next = *head;
    *head = newNode;
}

// Function to remove a UID from the list of logged-in users
void removeLoggedInUser(Node **head, const char *uid) {
    Node *current = *head;
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->uid, uid) == 0) {
            if (prev == NULL) {
                // If the node to be removed is the head
                *head = current->next;
            } else {
                prev->next = current->next;
            }

            // Free the memory allocated for the UID and the node
            free(current->uid);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }
}

// Function to check if a UID is logged in
int isUserLoggedIn(Node *head, const char *uid) {
    while (head != NULL) {
        if (strcmp(head->uid, uid) == 0) {
            return 1; // User is logged in
        }
        head = head->next;
    }
    return 0; // User is not logged in
}

// Function to free the memory allocated for the list
void freeList(Node *head) {
    Node *current = head;
    Node *next;

    while (current != NULL) {
        next = current->next;
        free(current->uid);
        free(current);
        current = next;
    }
}