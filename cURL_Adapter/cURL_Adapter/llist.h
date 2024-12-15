#ifndef CURL_LLIST_H
#define CURL_LLIST_H

#include <stddef.h>
#include <windows.h>

// Node structure for the doubly linked list
struct Curl_llist_node {
    void* ptr;                // Pointer to the stored data (can be HINTERNET or other types)
    struct Curl_llist_node* next;    // Pointer to the next node
    struct Curl_llist_node* prev;    // Pointer to the previous node
    struct Curl_llist* list;         // Pointer to the parent list
};

// Doubly linked list structure
struct Curl_llist {
    size_t size;                    // Number of nodes in the list
    struct Curl_llist_node* head;   // Pointer to the first node
    struct Curl_llist_node* tail;   // Pointer to the last node
    void (*dtor)(void* user, void* ptr); // Destructor function for node data
};

// Function declarations
void Curl_llist_init(struct Curl_llist* list, void (*dtor)(void* user, void* ptr));
void Curl_llist_append(struct Curl_llist* list, void* ptr, struct Curl_llist_node* node);
void Curl_llist_remove(struct Curl_llist* list, struct Curl_llist_node* node, void* user);
void Curl_llist_destroy(struct Curl_llist* list, void* user);
void Curl_llist_print(struct Curl_llist* list, void (*print_func)(void*));

#endif // CURL_LLIST_H
