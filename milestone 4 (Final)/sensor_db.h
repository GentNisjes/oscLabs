//
// Created by stijn on 12/14/24.
//

#ifndef SENSOR_DB_H
#define SENSOR_DB_H

#define CSV_NAME  "data.csv"

#include "sbuffer.h" // Include the sbuffer header for buffer operations
#include "config.h"  // Include configuration header for constants like CSV_NAME

// Define the structure for storage manager arguments
typedef struct {
    sbuffer_t *buffer;  // Shared buffer where data is stored
} storagemgr_args_t;

// Function declarations

// Function to handle data storage
// This function will be used to start the storage manager as a thread
int storagemgr(void* storage_args);

// Function to log process information (you can implement or replace this as needed)
void write_to_log_process(const char* message);

#endif // SENSOR_DB_H

