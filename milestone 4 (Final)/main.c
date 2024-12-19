#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "connmgr.h"    // Your connection manager header file
#include "datamgr.h"
#include "sbuffer.h"    // Your sbuffer header file
#include "sensor_db.h" // Storage manager header file (assumed)


typedef struct {
    int port;
    int max_conn;
    sbuffer_t *sbuffer;
} connmgr_args_t;

// Function to start the connection manager
void *start_connmgr(void *args) {
    connmgr_args_t *cm_args = (connmgr_args_t *)args;
    connmgr(cm_args->port, cm_args->max_conn, cm_args->sbuffer);
    return NULL;
}

// Function to start the storage manager
void *start_storagemgr(void *args) {
    storagemgr_args_t *sm_args = (storagemgr_args_t *)args;
    storagemgr(sm_args->buffer); // Assuming storagemgr takes the shared buffer as argument
    return NULL;
}

void *start_datamgr(void *args) {
    datamgr_args_t *dm_args = (datamgr_args_t *)args;
    datamgr(dm_args->buffer);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <max_connections>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int max_conn = atoi(argv[2]);

    printf("Starting connection manager on port %d, allowing up to %d connections...\n", port, max_conn);

    // Initialize the shared buffer
    sbuffer_t *sbuffer;
    if (sbuffer_init(&sbuffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer.\n");
        exit(EXIT_FAILURE);
    }

    // Create the connection manager thread
    pthread_t connmgr_thread;
    pthread_t datamgr_thread;
    pthread_t storagemgr_thread;

    connmgr_args_t cm_args = { port, max_conn, sbuffer };
    if (pthread_create(&connmgr_thread, NULL, start_connmgr, &cm_args) != 0) {
        fprintf(stderr, "Failed to create connection manager thread.\n");
        sbuffer_free(&sbuffer);  // Clean up in case of failure
        exit(EXIT_FAILURE);
    }

    datamgr_args_t dm_args = { sbuffer };
    if (pthread_create(&datamgr_thread, NULL, start_datamgr, &dm_args) != 0) {
        fprintf(stderr, "Failed to create connection manager thread.\n");
        sbuffer_free(&sbuffer);  // Clean up in case of failure
        exit(EXIT_FAILURE);
    }

    //sleep(1);
    printf("test________________________________");

    // Create the storage manager thread

    storagemgr_args_t sm_args = { sbuffer };
    if (pthread_create(&storagemgr_thread, NULL, start_storagemgr, &sm_args) != 0) {
        printf("Failed to create storage manager thread.\n");
        fprintf(stderr, "Failed to create storage manager thread.\n");
        sbuffer_free(&sbuffer);  // Clean up in case of failure
        pthread_cancel(connmgr_thread); // Cancel the connection manager thread if storage manager fails to start
        exit(EXIT_FAILURE);
    }
    printf("created storage manager thread.\n");

    // Wait for both threads to finish
    pthread_join(connmgr_thread, NULL);
    pthread_join(datamgr_thread, NULL);
    pthread_join(storagemgr_thread, NULL);

    // Cleanup: Free the shared buffer after both threads finish
    if (sbuffer_free(&sbuffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to free shared buffer.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connection manager and storage manager have exited. Shared buffer cleaned up.\n");
    return 0;
}
