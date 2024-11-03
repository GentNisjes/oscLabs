

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"




/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    if (*list == NULL) {
        *list = NULL; //out of safety, setting it again to null
        return;
    }

    dplist_node_t* current = (*list)->head;

    if (current == NULL) {
        free(*list);
        *list = NULL;
        return;
    }

    while (current != NULL) {
        dplist_node_t* next_node = current->next;

        // Free the element if needed
        // the value of the node could be empty,
        // in that case freeing the element would give problems
        // and ofc check the free element boolean
        if (free_element && current->element != NULL) {
            //element_free declared in the dplist struct
            //element_free needs to be called on a pointer of dplist type
            (*list)->element_free(&current->element);
        }

        // Free the current node
        free(current);
        current = next_node;
    }

    // Free the main list structure and set the pointer to NULL
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    //CODE from ex1/2 copied
    //only change: need for a deep copy or reference, based on the bool

    if (list == NULL) return NULL;
    dplist_node_t *list_node, *ref_at_index;;
    list_node = malloc(sizeof(dplist_node_t));

    if (insert_copy){
        //use the user specified "deep copy"
        list_node->element = list->element_copy(element);
    } else{
        //use a pointer reference to copy the element into the list
        list_node->element = element;
    }

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

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    if (list == NULL) {
        return NULL;  //If list is NULL, return NULL.
    }
    if (list->head == NULL) {
        return list;  //If the list is empty, return the unmodified list.
    }

    dplist_node_t* elementToRemove = dpl_get_reference_at_index(list, index);
    if (elementToRemove == NULL) {
        return list;
    }
    if (elementToRemove->prev == NULL) {    // Removing the head node
        list->head = elementToRemove->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
    } else {                                // Middle or last node
        elementToRemove->prev->next = elementToRemove->next;
        if (elementToRemove->next != NULL) {
            elementToRemove->next->prev = elementToRemove->prev;
        }
    }

    // Free the element if requested, bool check plus does the element contains something
    if (free_element && elementToRemove->element != NULL) {
        list->element_free(&elementToRemove->element);
    }

    free(elementToRemove);
    return list;
}

int dpl_size(dplist_t *list) {

    //check edge cases
    if (list==NULL){
        return -1;
    } else if (list->head == NULL) {
        return 0;
    }

    //normal case
    int count = 0;
    dplist_node_t *current = list->head;

    while (current != NULL) { // same as free fx, go through the list until the end
        count++;
        current = current->next; // Move to the next node
    }

    return count;

}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    // added check on node being null
    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    if (node == NULL) {
        return NULL;
    }
    return node->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {

    if (list==NULL || list->head == NULL)
    {
        return -1;
    }

    int index = 0;
    dplist_node_t *current = list->head;

    while (current != NULL) {
        if (list->element_compare(current->element, element) == 0) {
            return index;
        }
        if (current->next != NULL) {
            current = current->next;
            index ++;
        }
    }
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    if (list == NULL || list->head == NULL) { //either no list or empty list
        return NULL;
    }  else {
        dplist_node_t *current = list->head;
        int count = 0;

        if (index <= 0) {
            return current;
        }
        while (current->next != NULL && count < index) {
            current = current->next;
            count++;
        }
        return current;
    }
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    // If the list is empty, NULL is returned.
    // If 'list' is NULL, NULL is returned.
    // If 'reference' is NULL, NULL is returned.
    // If 'reference' is not an existing reference in the list, NULL is returned.
    if (list == NULL || list->head == NULL || reference == NULL || dpl_get_index_of_element(list, reference->element)) {
        return NULL;
    }
    return reference->element;

}


