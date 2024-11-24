//
// Created by stijn on 11/17/24.
//
#include <stdio.h>
#include "sensor_db.h"

#include "logger.h"

/*new log process is created in the
storage manager (sensor_db.c) as a child process, and a communication pipe between
the storage manager and the newly created log process is installed */


/* Opens the sensor database file */

// bool is an indication if the file should be overwritten
// if the file already exists or if the data should
// be appended to the existing file

FILE *open_db(char *filename, bool append) {
    //start up the child parent relation and thus the pipe between them
    //and opens up the gateaway log file
    create_log_process();

    //open up the sensor_db_file
    FILE *file = fopen(filename, append ? "a" : "w");
    if (file == NULL) {
        write_to_log_process("Error: Failed to open database file");
        perror("Failed to open database file");
        exit(EXIT_FAILURE);
    }

    write_to_log_process("Data file opened.");
    return file;
}

/* Inserts a sensor reading into the database file */
int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    //checking if fprintf executes as expected
    if (fprintf(f, "%u ,%.6f ,%ld\n", id, value, ts) < 0) {
        write_to_log_process("Failed to write to file");
        perror("Failed to write to file");
        return -1;
    }
    //log the successful writing to the file
    write_to_log_process("Data inserted.");
    return 0;
}

/* Closes the database file */
int close_db(FILE *f) {
    if (fclose(f) != 0) {
        write_to_log_process("Failed to close file");
        perror("Failed to close database file");
        return -1;
    }
    write_to_log_process("Data file closed.");

    //important to close the log pipe
    end_log_process();
    return 0;
    }





