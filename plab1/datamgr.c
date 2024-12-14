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

//test

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

// -------------------------------------------------------------------------------------------------

//both room sensor map and sensor data are created bby the file creator script

//ROOM SENSOR MAP - info

//room_sensor.map = READABLE
//just constant room info: room id and sensor id coupled with each other
//<room ID><space><sensor ID><\n>
//room ID and sensor ID are both positive 16-bit integers (uint16_t).

//SENSOR DATA - info

//sensor_data = NON READABLE - BINARY FILE
//<sensor ID><temperature><timestamp>...

dplist_t *list;

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data) {
    // Part 1: make dplist from sensor_map

    list = dpl_create(element_copy, element_free, element_compare);
    my_element_t temp_element;
    temp_element.indexLastAddedInRA = 0;
    temp_element.last_modified = 0;
    for (int i=0; i<RUN_AVG_LENGTH; i++) {
        temp_element.running_avg[i] = -555; // error checking number (check uninitialized values)
    }

    int index_dpl = 0;
    char line[12]; // 2x uint16 (max. 5 digits) + space + string terminator = 12 characters
    while (fgets(line, sizeof(line), fp_sensor_map)) {
        if (sscanf(line, "%hd %hd", &temp_element.room_id, &temp_element.id) == 2) {
            dpl_insert_at_index(list, &temp_element, index_dpl, true);
        }
        index_dpl++;
    }

    // Part 2: fill dplist with data from sensor_data

    sensor_data_t sensor_data; //defined in config.h
    // index_dpl gets reused, how fun

    while (!feof(fp_sensor_data)) {
        // Read the content as a sensor_data_t
        fread(&sensor_data.id, 2, 1, fp_sensor_data);
        fread(&sensor_data.value, 8, 1, fp_sensor_data);
        fread(&sensor_data.ts, 8, 1, fp_sensor_data);

        //replace id with 0 at end of file and break out of the while loop
        if (feof(fp_sensor_data)) {
            sensor_data.id = 0;
            break; // quit reading loop
        }

        // add values from binary file to the dplist
        index_dpl = dpl_get_index_of_element(list, &sensor_data);

        if (index_dpl == -1) {
            // values not in .map file should be dropped, with a log message saying so
            fprintf(stderr, "Sensor with that ID not in list\n");
        } else {
            //temp_node = dpl_get_reference_at_index(list, index_dpl);
            my_element_t *temp_node = (my_element_t *) dpl_get_element_at_index(list, index_dpl);
            temp_node->last_modified = sensor_data.ts;
            if (++temp_node->indexLastAddedInRA >= RUN_AVG_LENGTH) {
                temp_node->indexLastAddedInRA = 0;
            }
            temp_node->running_avg[temp_node->indexLastAddedInRA] = sensor_data.value;


        }
    }



}

void datamgr_free() {
    dpl_free(&list, true);
    free(list);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    int index_dpl;
    uint16_t temp_room = 0;

    // temporary extra node to store the ID in a my_element_t type to use it in the index_of_element method
    my_element_t* extra_node = malloc(sizeof(my_element_t)); // it's only a vessel to move the sensor id
    extra_node->id = sensor_id;

    // use the dpl_get_index_of_element from the dplist project
    index_dpl = dpl_get_index_of_element(list, extra_node);

    // check if "get index of element" was successful
    if (index_dpl == -1) {
        fprintf(stderr, "Sensor with that ID not in list\n");
    } else {
        my_element_t* temp_node = (my_element_t *) dpl_get_element_at_index(list, index_dpl);
        temp_room = temp_node->room_id;
    }

    // cleanup used extra node for reuse and memory sanity reasons
    free(extra_node);
    extra_node = NULL;

    return temp_room;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    double average = 0;
    my_element_t search_node;
    search_node.id = sensor_id;

    // Find the index of the sensor in the list
    int index_dpl = dpl_get_index_of_element(list, &search_node);
    if (index_dpl == -1) {
        fprintf(stderr, "Sensor with ID %d not in list\n", sensor_id); // "Sensor with that ID not in list\n"
    }

    // Retrieve the actual node from the list
    my_element_t* temp_node = (my_element_t *) dpl_get_element_at_index(list, index_dpl);

    // Calculate the running average of valid values
    double sum = 0;
    int valid_count = 0;
    for (int i = 0; i < RUN_AVG_LENGTH; i++) {
        if (temp_node->running_avg[i] > -275) { // Valid data check
            sum += temp_node->running_avg[i];
            valid_count++;
        }
    }

    if (valid_count == 0) {
        fprintf(stderr, "No valid data for sensor ID %d\n", sensor_id);
    }

    average = sum / valid_count;

    // Display the average temperature
    printf("Temp: %lf\n", average);
    if (average > SET_MAX_TEMP) {
        printf("It's too warm\n");
    } else if (average < SET_MIN_TEMP) {
        printf("It's too cold\n");
    }

    return average;
}


time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    int index_dpl;
    time_t temp_time = 0;

    my_element_t* extra_node = malloc(sizeof(my_element_t));
    extra_node->id = sensor_id;

    index_dpl = dpl_get_index_of_element(list, extra_node);

    if (index_dpl == -1) {
        fprintf(stderr, "Sensor with that ID not in list\n");
    } else {
        my_element_t* temp_node = (my_element_t *) dpl_get_element_at_index(list, index_dpl);
        temp_time = temp_node->last_modified;
    }

    free(extra_node);
    extra_node = NULL;
    return temp_time;
}

int datamgr_get_total_sensors() {
    return dpl_size(list);
}