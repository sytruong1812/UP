#include <stdio.h>
#include <stdlib.h>

#include "llist.h"

// Initialize the linked list
void Curl_llist_init(struct Curl_llist* list, void (*dtor)(void* user, void* ptr)) {
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->dtor = dtor;
}

// Add a new node to the end of the list
void Curl_llist_append(struct Curl_llist* list, void* ptr, struct Curl_llist_node* node) {
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for list node\n");
        return;
    }
    node->ptr = ptr;
    node->next = NULL;
    node->prev = list->tail;
    node->list = list;

    if (list->tail) {
        list->tail->next = node;
    }
    else {
        list->head = node; // First element in the list
    }
    list->tail = node;
    list->size++;
}

// Remove a node from the list
void Curl_llist_remove(struct Curl_llist* list, struct Curl_llist_node* node, void* user) {
    if (!node) return;

    if (node->prev) {
        node->prev->next = node->next;
    }
    else {
        list->head = node->next; // Removing the head node
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
    else {
        list->tail = node->prev; // Removing the tail node
    }

    if (list->dtor)
        list->dtor(user, node->ptr);

    free(node);
    list->size--;
}

// Destroy the entire list and free memory
void Curl_llist_destroy(struct Curl_llist* list, void* user) {
    struct Curl_llist_node* current = list->head;
    while (current) {
        struct Curl_llist_node* next = current->next;
        if (list->dtor)
            list->dtor(user, current->ptr);
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Print the list (for debugging purposes)
void Curl_llist_print(struct Curl_llist* list, void (*print_func)(void*)) {
    struct Curl_llist_node* current = list->head;
    while (current) {
        print_func(current->ptr);
        current = current->next;
    }
}

