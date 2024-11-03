#define _GNU_SOURCE



#include "dplist.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
    int id;
    char* name;
} my_element_t;

void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);

void * element_copy(void * element) {
    my_element_t* copy = malloc(sizeof (my_element_t));
    char* new_name;
    asprintf(&new_name,"%s",((my_element_t*)element)->name); //asprintf requires _GNU_SOURCE
    assert(copy != NULL);
    copy->id = ((my_element_t*)element)->id;
    copy->name = new_name;
    return (void *) copy;
}

void element_free(void ** element) {
    free((((my_element_t*)*element))->name);
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);
}

void ck_assert_msg(bool result, char * msg){
    if(!result) printf("%s\n", msg);
}

void yourtest1(){

    // Test free NULL, don't use callback
    dplist_t *list = NULL;
    dpl_free(&list, false);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");


    // Test free NULL, use callback
    list = NULL;
    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");


    // Test free empty list, don't use callback
    list = dpl_create(element_copy, element_free, element_compare);
    dpl_free(&list, false);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");


    // Test free empty list, use callback
    list = dpl_create(element_copy, element_free, element_compare);
    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "Failure: expected result to be NULL");


    // Test adding node and removing it, in the boundaries of the list
    list = dpl_create(element_copy, element_free, element_compare);
    my_element_t element1 = {1, "Element1"};
    dpl_insert_at_index(list, &element1, 0, true);
    ck_assert_msg(dpl_size(list) == 1, "Failure: expected list size to be 1 after insertion.");

    dpl_remove_at_index(list, 0, true);
    ck_assert_msg(dpl_size(list) == 0, "Failure: expected list size to be 0 after removal.");

    dpl_free(&list, true);
    ck_assert_msg(list == NULL, "Failure: expected list to be NULL after free.");

    //______________________________________________________________________________________

    // Test adding node and removing on an index that is non-existent
    dplist_t *list2 = dpl_create(element_copy, element_free, element_compare);
    //my_element_t element1 = {1, "Element1"};
    dpl_insert_at_index(list2, &element1, 0, true);

    dplist_t *unchanged_list = dpl_remove_at_index(list, 5, true); // Out-of-bounds index
    ck_assert_msg(unchanged_list == list, "Failure: list should remain unchanged after out-of-bounds removal.");

    dpl_free(&list, true);

    //______________________________________________________________________________________

    // Test adding node at negative index
    dplist_t *list3 =  dpl_create(element_copy, element_free, element_compare);
    dpl_insert_at_index(list3, &element1, -5, true);
    int indexElement = dpl_get_index_of_element(list3, &element1);
    ck_assert_msg(indexElement == 0, "Failure: elements inserted at negative indexes should be added as the first node in the list.");
    dpl_free(&list3, true);

    // Test removing a node at a negative index
    dplist_t *list4 = dpl_create(element_copy, element_free, element_compare);
    my_element_t element2 = {2, "Element2"};
    dpl_insert_at_index(list4, &element2, 0, true);

    list4 = dpl_remove_at_index(list4, -3, true);
    ck_assert_msg(dpl_size(list4) == 0, "Failure: removing a node at a negative index should remove the first node in the list.");

    // Clean up
    dpl_free(&list4, true);

    //______________________________________________________________________________________

    // Test adding a node at an existing index
    dplist_t *list5 = dpl_create(element_copy, element_free, element_compare);
    my_element_t element3 = {3, "Element3"};
    my_element_t element4 = {4, "Element4"};
    my_element_t element5 = {5, "Element5"};
    my_element_t new_element = {6, "NewElement"};

    // Insert the normal elements into the list
    dpl_insert_at_index(list5, &element3, 0, true);
    dpl_insert_at_index(list5, &element4, 1, true);
    dpl_insert_at_index(list5, &element5, 2, true);

    // Insert the new element at index 1 (between element3 and element4)
    dpl_insert_at_index(list5, &new_element, 1, true);

    // Check that the new element is at index 1
    void *element_at_index_1 = dpl_get_element_at_index(list5, 1);
    ck_assert_msg(element_compare(element_at_index_1, &new_element) == 0, "Failure: new element not found at the expected index.");

    // Check the order of the other elements
    void *element_at_index_2 = dpl_get_element_at_index(list5, 2);
    void *element_at_index_3 = dpl_get_element_at_index(list5, 3);
    ck_assert_msg(element_compare(element_at_index_2, &element4) == 0, "Failure: element at index 2 is not as expected.");
    ck_assert_msg(element_compare(element_at_index_3, &element5) == 0, "Failure: element at index 3 is not as expected.");

    // Clean up
    dpl_free(&list5, true);

}




int main(void) {

    yourtest1();
    return 0;
}
