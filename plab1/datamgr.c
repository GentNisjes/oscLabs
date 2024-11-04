//
// Created by stijn on 11/4/24.
//

#include "datamgr.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "lib/dplist.h"

//callback functions, from milestone 1
//adapted to this context

void * element_copy(void * element) {
    my_element_t *copy = malloc(sizeof(my_element_t));
    assert(copy != NULL); //check if correctly allocated by malloc

    //now that the copy is made we can take the components from the void type
    //and put it in the new my_element_t type copy
    copy->id = ((my_element_t *)element)->id;
    copy->room_id = ((my_element_t *)element)->room_id;
    copy->last_modified = ((my_element_t *)element)->last_modified;
    copy->indexLastAddedInRA = ((my_element_t *)element)->indexLastAddedInRA;
    for (int i=0; i<RUN_AVG_LENGTH; i++) {
        //copy all the elements from the running_avg array
        copy->running_avg[i] = ((my_element_t *)element)->running_avg[i];
    }

    //cast back to void, since that's the return type
    return (void *)copy;
}

void element_free(void ** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->id < ((my_element_t*)y)->id) ? -1 : (((my_element_t*)x)->id == ((my_element_t*)y)->id) ? 0 : 1);

    //---> quite hard to read, this is the written out version of the compare callback function
    // my_element_t *element_x = (my_element_t *)x;
    // my_element_t *element_y = (my_element_t *)y;
    //
    // if (element_x->id < element_y->id) {
    //     return -1;
    // } else if (element_x->id == element_y->id) {
    //     return 0;
    // } else {
    //     return 1;
    // }
}

//both room sensor map and sensor data are created bby the file creator script

//ROOM SENSOR MAP - info

//room_sensor.map = READABLE
//just constant room info: room id and sensor id coupled with each other
//<room ID><space><sensor ID><\n>
//room ID and sensor ID are both positive 16-bit integers (uint16_t).

//SENSOR DATA - info

//sensor_data = NON READABLE - BINARY FILE
//<sensor ID><temperature><timestamp>...


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