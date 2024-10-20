/**
 * \author Jeroen Van Aken, Bert Lagaisse, Ludo Bruynseels
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "dplist.h"



/*
 * The real definition of struct list / struct node
 */

// declare a structure for the node: pointer to prev, to next and the value
struct dplist_node {
    dplist_node_t *prev, *next;
    element_t element;
};

// declaration of a struct for the head of the linked list
struct dplist {
    dplist_node_t *head;
    // more fields will be added later
};


dplist_t* dpl_create() {
    dplist_t *list;     //head node of the linked list
    list = malloc(sizeof(struct dplist));   //assign storage to head node (=pointer)
    list->head = NULL;  //set the first node value to NULL (indicator for end of the string)
  return list;
}

/** Deletes all elements in the list
 * - Every list node of the list must be deleted. (free memory)
 * - The list itself also needs to be deleted. (free all memory)
 * - '*list' must be set to NULL.
 * \param list a double pointer to the list
 */
void dpl_free(dplist_t **list) {

    //TODO: add your code here
    dplist_node_t* current = (*list)->head;

    if (current == NULL) {
        printf("linked list is empty");
    }


    while(current != NULL) {
        dplist_node_t* next_node = current->next;
        free(current);
        current = next_node;
    }

    free(*list);
    *list = NULL;

    //Do extensive testing with valgrind. 

}

/* Important note: to implement any list manipulation operator (insert, append, delete, sort, ...), always be aware of the following cases:
 * 1. empty list ==> avoid errors
 * 2. do operation at the start of the list ==> typically requires some special pointer manipulation
 * 3. do operation at the end of the list ==> typically requires some special pointer manipulation
 * 4. do operation in the middle of the list ==> default case with default pointer manipulation
 * ALWAYS check that you implementation works correctly in all these cases (check this on paper with list representation drawings!)
 **/


dplist_t *dpl_insert_at_index(dplist_t *list, element_t element, int index) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));

    list_node->element = element;
    // pointer drawing breakpoint

    //when there's no node yet, so next and prev to null
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    //insert the new node at the start of the list
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    //insert the new node at the end of the list
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);

        // pointer drawing breakpoint
        //insert the new node in the middle of the list
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint

        //insert the new node at the end
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    return NULL;
}

int dpl_size(dplist_t *list) {

    //TODO: add your code here
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    //int count = 0 ;
    if (index <= 0) {

    } else if()
    dplist_node_t *dummy = NULL;

    //TODO: add your code here
    return dummy;
}

element_t dpl_get_element_at_index(dplist_t *list, int index) {

    //TODO: add your code here
    return '\e';
}

int dpl_get_index_of_element(dplist_t *list, element_t element) {

    //TODO: add your code here
    return -1;
}



