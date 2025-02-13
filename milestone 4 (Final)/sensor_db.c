//
// Created by stijn on 12/14/24.
//

#include "sensor_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "sbuffer.h"
#include "sensor_db.h"

int storagemgr(void* storage_args) {
    storagemgr_args_t* args = (storagemgr_args_t*)storage_args;
    char logmsg[LOG_MESSAGE_LENGTH];

    // Open CSV file for writing
    FILE* csv = fopen(CSV_NAME, "w"); // "w" mode to create/overwrite the file
    if (csv == NULL) {
        write_to_log_process("Error opening " CSV_NAME ", shutting down storage manager");
        return 1;  // Error opening the file
    }
    write_to_log_process("A new " CSV_NAME " file has been created.");

    // Sensor data structure for storing incoming data
    sensor_data_t received_data;

    while (1) {
        // Get data from the buffer
        if (sbuffer_remove(args->buffer, &received_data, 2) == 0) {
            // sprintf(logmsg, "Data removed from buffer - Sensor ID: %d, Temp: %lf, Timestamp: %ld\n", received_data.id, received_data.value, received_data.ts);
            // write_to_log_process(logmsg);
            // Check if the received data is valid (id != 0)
            if (received_data.id != 0) {
                // Write the data to the CSV file
                fprintf(csv, "%d, %lf, %ld\n", received_data.id, received_data.value, received_data.ts);
                fflush(csv);

                // Log the successful insertion
                sprintf(logmsg, "Data insertion from sensor %u succeeded.", received_data.id);
                write_to_log_process(logmsg);
            } else {
                // Exit if the data is invalid or indicates no more data
                // sprintf(logmsg, "ZERO ID: %d, %lf, %ld\n", received_data.id, received_data.value, received_data.ts);
                // write_to_log_process(logmsg);
                break;
            }
        }
        // else {
        //     // Log a message if buffer is empty or no data available within the timeout period
        //     //write_to_log_process("Storage MGR: Buffer empty or no data available within timeout.");
        //     write_to_log_process("Storage MGR: Buffer empty or no data available within timeout. \n");
        // }
    }

    write_to_log_process("Attempting to close the file...");
    // Close the CSV file when done
    if (fclose(csv) != 0) {write_to_log_process("Error closing " CSV_NAME );}
    write_to_log_process("The " CSV_NAME " file has been closed.");
    return 0;
}
