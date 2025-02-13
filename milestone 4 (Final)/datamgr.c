#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sbuffer.h"
#include "config.h"
#include "lib/dplist.h"
#include "datamgr.h"

#include "sensor_db.h"


dplist_t *list;

int datamgr(void* data_args) {
    datamgr_args_t* args = (datamgr_args_t*)data_args;
    char logmsg[LOG_MESSAGE_LENGTH];

    // make dplist from sensor_map
    list = dpl_create(element_copy, element_free, element_compare);
    if (datamgr_parse_map(SENSOR_MAP) != 0) {
        write_to_log_process("Error parsing" SENSOR_MAP ", shutting down data manager");
        return 1;
    }
    write_to_log_process("Finished parsing " SENSOR_MAP);

    sensor_data_t received_data;
    my_element_t* temp_node;
    int index_dpl;

    while (1) {
        // get data from buffer
        if (sbuffer_read(args->buffer, &received_data, 1) == 0) {
            //write_to_log_process("Data MGR: read done correctly");
            if (received_data.id != 0) {
                printf("%lu: %d, %lf, %ld\n", pthread_self(), received_data.id, received_data.value, received_data.ts);

                // add values to the dplist
                index_dpl = dpl_get_index_of_element(list, &received_data);
                if (index_dpl == -1) {
                    // incorrect sensor id, not in the map file
                    sprintf(logmsg, "Received sensor data with invalid sensor node ID %u", received_data.id);
                    write_to_log_process(logmsg);
                } else {
                    temp_node = (my_element_t *) dpl_get_element_at_index(list, index_dpl);
                    temp_node->last_modified = received_data.ts;
                    if (++temp_node->ra_lastadded >= RUN_AVG_LENGTH) {
                        temp_node->ra_lastadded = 0;
                    }
                    temp_node->running_avg[temp_node->ra_lastadded] = received_data.value;

                    datamgr_get_avg(temp_node);
                }
            } else { // end of buffer
                //write_to_log_process("Data MGR: received id is zero");
                datamgr_free();
                break;
            }
        } else {
            //write_to_log_process("Data MGR: read went wrong\n");
        }
    }
    return 0;
}

int datamgr_parse_map(char* sensor_map) {
    FILE* map = fopen(sensor_map, "r");
    if (map == NULL) {
        return 1;
    }

    char line[12];
    my_element_t temp_element;

    temp_element.ra_lastadded = 0;
    temp_element.last_modified = 0;
    for (int i=0; i<RUN_AVG_LENGTH; i++) {
        temp_element.running_avg[i] = -300; // set temp to be physically impossible, so it won't be counted in the average
    }

    int index_dpl = 0;
    while (fgets(line, sizeof(line), map)) {
        if (sscanf(line, "%hd %hd", &temp_element.room_id, &temp_element.id) != 2) {
            write_to_log_process("Unexpected line reading from " SENSOR_MAP);
        } else {
            if (dpl_insert_at_index(list, &temp_element, index_dpl, true) == NULL) {
                return 1;
            }
        }
        index_dpl++;
    }
    // close map file
    if (fclose(map) != 0) {
        write_to_log_process("Error closing " SENSOR_MAP ", bummer");
    }
    return 0;
}

void datamgr_free() {
    dpl_free(&list, true);
    free(list);
}

sensor_value_t datamgr_get_avg(my_element_t* node) {
    double average = 0;
    char logmsg[LOG_MESSAGE_LENGTH];
    for (int i=0; i<RUN_AVG_LENGTH; i++) {
        if (node->running_avg[i] > -275) { // check if the read temp is physically realistic (not below the absolute zero 0K or -275°C)
            average += node->running_avg[i];
        }
    }
    average = average/RUN_AVG_LENGTH;
    if (average > SET_MAX_TEMP) {
        sprintf(logmsg, "Sensor node %u reports it’s too hot (avg temp = %lf)", node->id, average);
        write_to_log_process(logmsg);
    } else if (average < SET_MIN_TEMP) {
        sprintf(logmsg, "Sensor node %u reports it’s too cold (avg temp = %lf)", node->id, average);
        write_to_log_process(logmsg);
    }
    return average;
}

int datamgr_get_total_sensors() {
    return dpl_size(list);
}

void *element_copy(void *element)
{
    my_element_t *copy = malloc(sizeof(my_element_t));

    assert(copy != NULL);
    copy->id = ((my_element_t *)element)->id;
    copy->room_id = ((my_element_t *)element)->room_id;
    copy->last_modified = ((my_element_t *)element)->last_modified;
    copy->ra_lastadded = ((my_element_t *)element)->ra_lastadded;
    for (int i=0; i<RUN_AVG_LENGTH; i++) {
        copy->running_avg[i] = ((my_element_t *)element)->running_avg[i];
    }
    return (void *)copy;
}

void element_free(void **element)
{
    free(*element);
    *element = NULL;
}

int element_compare(void *x, void *y) // returns 0 if id is equal
{
    return ((((my_element_t *)x)->id < ((my_element_t *)y)->id) ? -1 : (((my_element_t *)x)->id == ((my_element_t *)y)->id) ? 0
                                                                                                                            : 1);
}