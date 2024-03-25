#include "linked_list.h"

// Creates and returns a new list
list_t* list_create()
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Destroys a list
void list_destroy(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}

// Returns beginning of the list
list_node_t* list_begin(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return 0;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Inserts a new node in the list with the given data
void list_insert(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}

// Executes a function for each element in the list
void list_foreach(list_t* list, void (*func)(void* data))
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}
