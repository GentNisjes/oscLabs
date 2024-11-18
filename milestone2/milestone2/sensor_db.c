//
// Created by stijn on 11/17/24.
//
#include <stdio.h>
#include "sensor_db.h"

/*new log process is created in the
storage manager (sensor_db.c) as a child process, and a communication pipe between
the storage manager and the newly created log process is installed */


/* Opens the sensor database file */

// bool is an indication if the file should be overwritten
// if the file already exists or if the data should
// be appended to the existing file

FILE *open_db(char *filename, bool append) {
    FILE *file = fopen(filename, append ? "a" : "w");
    if (file == NULL) {
        perror("Failed to open database file");
        exit(EXIT_FAILURE);
    }
    return file;
}

/* Inserts a sensor reading into the database file */
int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (fprintf(f, "%u ,%.6f ,%ld\n", id, value, ts) < 0) {
        perror("Failed to write to database");
        return -1;
    }
    return 0;
}

/* Closes the database file */
int close_db(FILE *f) {
    if (fclose(f) != 0) {
        perror("Failed to close database file");
        return -1;
    }
    return 0;
}

