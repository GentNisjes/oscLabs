//
// Created by stijn on 12/6/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "sbuffer.h"
#include "config.h"
#include "main.h"

// Global mutex for writing to the CSV file
pthread_mutex_t csv_mutex; // make an extra mutex for the readers

FILE* csv;

int main() {
    pthread_mutex_init(&csv_mutex, NULL);
    // Allocate memory for the buffer pointer and initialize it
    // done through the sbuffer_init function
    sbuffer_t* myBuffer;
    if (sbuffer_init(&myBuffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize buffer.\n");
        return EXIT_FAILURE;
    }

    // Open the output CSV file
    csv = fopen("sensor_data_out.csv", "w");
    if (!csv) {
        perror("Failed to open sensor_data_out.csv");
        sbuffer_free(&myBuffer);
        return EXIT_FAILURE;
    }
    printf("file successfully opened\n");

    // Create reader threads
    // and check both thread creations, if set up correctly
    pthread_t thread1, thread2;
    if (pthread_create(&thread1, NULL, reader, myBuffer) != 0) {
        perror("Failed to create thread1");
        fclose(csv);
        sbuffer_free(&myBuffer);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread2, NULL, reader, myBuffer) != 0) {
        perror("Failed to create thread2");
        fclose(csv);
        sbuffer_free(&myBuffer);
        return EXIT_FAILURE;
    }

    // Open the input sensor data file
    FILE *fpsd = fopen("sensor_data", "r");
    if (!fpsd) {
        perror("Failed to open sensor_data");
        fclose(csv);
        sbuffer_free(&myBuffer);
        return EXIT_FAILURE;
    }

    sensor_data_t sensor_data;

    // Read and insert sensor data
    // while still checking if the read operation was handled correctly
    while (fread(&sensor_data.id, sizeof(sensor_data.id), 1, fpsd) == 1 &&
           fread(&sensor_data.value, sizeof(sensor_data.value), 1, fpsd) == 1 &&
           fread(&sensor_data.ts, sizeof(sensor_data.ts), 1, fpsd) == 1) {

        sbuffer_insert(myBuffer, &sensor_data);
        printf("%d, %lf, %ld\n", sensor_data.id, sensor_data.value, sensor_data.ts);
        usleep(10000); // 10 milliseconds delay
    }

    // Insert end-of-stream markers for each reader thread
    for (int i = 0; i < 2; i++) {
        sensor_data.id = 0;
        sbuffer_insert(myBuffer, &sensor_data);
    }

    // Wait for reader threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    fclose(fpsd);
    fclose(csv);
    sbuffer_free(&myBuffer);

    pthread_mutex_destroy(&csv_mutex);


    return EXIT_SUCCESS;
}

// Reader function
// used inside the pthreads
void* reader(void* buffer) {
    sensor_data_t received_data;

    while (true) {
        usleep(25000);

        if (sbuffer_remove(buffer, &received_data) == SBUFFER_SUCCESS) {
            if (received_data.id == 0) {
                break; // End-of-stream marker received
            }

            // Write to the CSV file with mutual exclusion
            pthread_mutex_lock(&csv_mutex);
            printf("%lu: %d, %lf, %ld\n", pthread_self(), received_data.id, received_data.value, received_data.ts);
            fprintf(csv, "%d, %lf, %ld\n", received_data.id, received_data.value, received_data.ts);
            pthread_mutex_unlock(&csv_mutex);
        }
    }

    return NULL;
}







