/**
 * \author {AUTHOR}
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#define SET_MAX_TEMP 50.0
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 0.0
#endif

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)

//the main idea for the running_avg array is to create a circular buffer
//with pointers to the specific temperatures in the temperatures map
//we get this circularity by doing the following:
//----------> every time a new temperature is added to the running avg
//            we increment a counter, indexLastAddedInRA
//
//----------> If indexLastAddedInRA exceeds RUN_AVG_LENGTH - 1
//            (i.e., goes beyond the end of the array),
//            it wraps around to 0, ensuring that the array acts as a circular buffer.
//            (so the counter is then used to insert at an index = counter)
//
//----------> This results in a circular updated array
//            which is suitable for all lengths of data inputs

typedef struct{
   uint16_t id;                               //given variable, declared in the document
   uint16_t room_id;                          //given variable, declared in the document
   double running_avg[RUN_AVG_LENGTH];        //array of pointers pointing to the last x temperatures
   int indexLastAddedInRA;                    //index of newly added last temperature reading
   time_t last_modified;                      //given variable, declared in the document
} my_element_t;

void *element_copy(void *element);
void element_free(void **element);
int element_compare(void *x, void *y);

/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();

#endif  //DATAMGR_H_
