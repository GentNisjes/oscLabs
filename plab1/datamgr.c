//
// Created by stijn on 11/4/24.
//

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "lib/dplist.h"

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



// //callback functions
// void * element_copy(void * element) {
//     element* copy = malloc(sizeof (element));
//     char* new_name;
//     asprintf(&new_name,"%s",((my_element_t*)element)->name); //asprintf requires _GNU_SOURCE
//     assert(copy != NULL);
//     copy->id = ((my_element_t*)element)->id;
//     copy->name = new_name;
//     return (void *) copy;
// }
//
// void element_free(void ** element) {
//     free((((my_element_t*)*element))->name);
//     free(*element);
//     *element = NULL;
// }
//
// int element_compare(void * x, void * y) {
//     return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);


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

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {

}

void datamgr_free() {

}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {

}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {

}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {

}

int datamgr_get_total_sensors() {

}