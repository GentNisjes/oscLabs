// // //
// // // Created by stijn on 11/17/24.
// // //
// // #include <stdio.h>
// // #include "sensor_db.h"
// //
// // #include "logger.h"
// //
// // /*new log process is created in the
// // storage manager (sensor_db.c) as a child process, and a communication pipe between
// // the storage manager and the newly created log process is installed */
// //
// //
// // /* Opens the sensor database file */
// //
// // // bool is an indication if the file should be overwritten
// // // if the file already exists or if the data should
// // // be appended to the existing file
// //
// // FILE *open_db(char *filename, bool append) {
// //     //start up the child parent relation and thus the pipe between them
// //     //and opens up the gateaway log file
// //     create_log_process();
// //
// //     //open up the sensor_db_file
// //     FILE *file = fopen(filename, append ? "a" : "w");
// //     if (file == NULL) {
// //         write_to_log_process("Error: Failed to open database file");
// //         perror("Failed to open database file");
// //         exit(EXIT_FAILURE);
// //     }
// //
// //     write_to_log_process("Data file opened.");
// //     return file;
// // }
// //
// // /* Inserts a sensor reading into the database file */
// // int insert_sensor(FILE *f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
// //     //checking if fprintf executes as expected
// //     if (fprintf(f, "%u ,%.6f ,%ld\n", id, value, ts) < 0) {
// //         write_to_log_process("Failed to write to file");
// //         perror("Failed to write to file");
// //         return -1;
// //     }
// //     //log the successful writing to the file
// //     write_to_log_process("Data inserted.");
// //     return 0;
// // }
// //
// // /* Closes the database file */
// // int close_db(FILE *f) {
// //     if (fclose(f) != 0) {
// //         write_to_log_process("Failed to close file");
// //         perror("Failed to close database file");
// //         return -1;
// //     }
// //     write_to_log_process("Data file closed.");
// //
// //     //important to close the log pipe
// //     end_log_process();
// //     return 0;
// //     }
// //
//
// #include "sensor_db.h"
// #include "logger.h"
//
// //TODO: use better log messages
//
// FILE * open_db(char * filename, bool append) {
//     create_log_process();
//     FILE* csv;
//
//     if (!append) {
//         csv = fopen(filename, "w"); // replace file
//     } else {
//         csv = fopen(filename, "a"); //append to existing file
//     }
//     write_to_log_process("Data file opened.");
//
//     return csv;
// }
//
// int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
//     fprintf(f, "%d, %lf, %ld\n", id, value, ts);
//     write_to_log_process("Data inserted.");
//     return 0;
// }
//
// int close_db(FILE * f){
//     fclose(f);
//     write_to_log_process("Data file closed.");
//     end_log_process();
//     return 0;
// }

//
// Created by student on 11/18/24.
//
#include "sensor_db.h"

#include <errno.h>
#include <signal.h>

#include "logger.h"
//To make the code a bit more robust.
// It's still the responsability of the caller to open the db and so forth, but it will now return an error when inserting a sensor or closing a db with no file open.
bool some_db_open = false;
bool multiple_db_open = false;
FILE* open_db(const char* filename, bool append) {
    create_log_process();
    some_db_open = true;
    if (some_db_open)
    {
        multiple_db_open = true; //this basically turns off the protection that some_db_open gives.
    }
    const char* open_options;
    if (append)
    {
        open_options = "a";
    } else
    {
        open_options = "w";
    }
    FILE* fptr = fopen(filename, open_options);
    if (fptr == NULL)
    {
        write_to_log_process("Could not open file");
        exit(1);
    }
    write_to_log_process("Data file opened.");
    return fptr;
}


int insert_sensor(FILE* f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    if (f == NULL)
    {
        return -1;
    }
    int result = fprintf(f, "%hu, %f, %ld\n", id, value, ts);
    if (result < 0)
    {
        write_to_log_process("Could not write to sensor file");
        return errno;
    }
    write_to_log_process("Data inserted.");
    return 0;
}

int close_db(FILE* f)
{
    if (f == NULL)
    {
        return -1;
    }
    int result = fclose(f);
    if (result < 0)
    {
        write_to_log_process("Could not close sensor file");
        return errno;
    }
    write_to_log_process("Data file closed.");
    end_log_process();
    return 0;
}
