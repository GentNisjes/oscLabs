#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>

#include "connmgr.h"    // connection manager header file
#include "datamgr.h"
#include "sbuffer.h"    // sbuffer header file
#include "sensor_db.h" // Storage manager header file

#define LOG_FILE_NAME  "gateway.log"

int fd1[2];
pid_t pid;
int logcounter;
FILE* log_file;

typedef struct {
    int port;
    int max_conn;
    sbuffer_t **sbuffer;
} connmgr_args_t;

// Function to start the connection manager
void *start_connmgr(void *args) {
    connmgr_args_t *cm_args = (connmgr_args_t *)args;
    connmgr(cm_args->port, cm_args->max_conn, *(cm_args->sbuffer));
    return NULL;
}

void *start_storagemgr(void *args) {
    storagemgr_args_t *sm_args = (storagemgr_args_t *)args;
    storagemgr(sm_args->buffer);
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

    // create logger child thread
    int result = 0;
    if (create_log_process() != 0) {
        printf("Error opening log process, shutting down");
        return 1;
    }
    if (pid == 0) {
        // Child process
        while (result == 0) {
            result = log_pipe_to_file();
        }
        end_log_process();
        return 0;
    }
    // parent process
    write_to_log_process("Log process started");

    int port = atoi(argv[1]);
    int max_conn = atoi(argv[2]);

    printf("Starting connection manager on port %d, allowing up to %d connections...\n", port, max_conn);

    // Initialize the shared buffer
    sbuffer_t **shared_data = malloc(sizeof(sbuffer_t *));
    if (shared_data == NULL) {
        write_to_log_process("Error allocating memory to buffer, shutting down");
        return 1;
    }

    if (sbuffer_init(shared_data) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer.\n");
        free(shared_data);
        exit(EXIT_FAILURE);
    }

    // Pass arguments to manager threads
    connmgr_args_t *conn_args = malloc(sizeof(connmgr_args_t));
    datamgr_args_t *data_args = malloc(sizeof(datamgr_args_t));
    storagemgr_args_t *storage_args = malloc(sizeof(storagemgr_args_t));

    if (!conn_args || !data_args || !storage_args) {
        fprintf(stderr, "Failed to allocate memory for arguments.\n");
        sbuffer_free(shared_data);
        free(*shared_data);
        free(shared_data);
        exit(EXIT_FAILURE);
    }

    // Initialize the arguments for each manager
    conn_args->port = port;
    conn_args->max_conn = max_conn;
    conn_args->sbuffer = shared_data;

    data_args->buffer = *shared_data;

    storage_args->buffer = *shared_data;

    // Create the connection manager thread
    pthread_t connmgr_thread;
    if (pthread_create(&connmgr_thread, NULL, start_connmgr, conn_args) != 0) {
        fprintf(stderr, "Failed to create connection manager thread.\n");
        sbuffer_free(shared_data);
        free(*shared_data);
        free(shared_data);
        free(conn_args);
        free(data_args);
        free(storage_args);
        exit(EXIT_FAILURE);
    }

    // Create the data manager thread
    pthread_t datamgr_thread;
    if (pthread_create(&datamgr_thread, NULL, (void*)datamgr, data_args) != 0) {
        fprintf(stderr, "Failed to create data manager thread.\n");
        sbuffer_free(shared_data);
        pthread_cancel(connmgr_thread);
        free(*shared_data);
        free(shared_data);
        free(conn_args);
        free(data_args);
        free(storage_args);
        exit(EXIT_FAILURE);
    }

    // Create the storage manager thread
    pthread_t storagemgr_thread;
    if (pthread_create(&storagemgr_thread, NULL, (void*)storagemgr, storage_args) != 0) {
        fprintf(stderr, "Failed to create storage manager thread.\n");
        sbuffer_free(shared_data);
        pthread_cancel(connmgr_thread);
        pthread_cancel(datamgr_thread);
        free(*shared_data);
        free(shared_data);
        free(conn_args);
        free(data_args);
        free(storage_args);
        exit(EXIT_FAILURE);
    }

    // Wait for the threads to finish
    pthread_join(connmgr_thread, NULL);
    pthread_join(datamgr_thread, NULL);
    pthread_join(storagemgr_thread, NULL);

    // Free dynamically allocated memory
    free(conn_args);
    free(data_args);
    free(storage_args);

    // Cleanup the shared buffer
    if (sbuffer_free(shared_data) != SBUFFER_SUCCESS) {
        write_to_log_process("Failed to free shared buffer.\n");
        free(*shared_data); // Free the actual buffer
        free(shared_data);  // Free the double pointer
        exit(EXIT_FAILURE);
    }

    free(*shared_data); // Free the actual buffer
    free(shared_data);  // Free the double pointer

    // end logger thread
    write_to_log_process("Shutting down - log process closed");
    end_log_process();
    return 0;
}

pthread_mutex_t pipemutex;
int write_to_log_process(char *msg){
    pthread_mutex_lock(&pipemutex);
    write(fd1[1], msg, LOG_MESSAGE_LENGTH);
    pthread_mutex_unlock(&pipemutex);
    return 0;
}

int log_pipe_to_file() {
    time_t now;
    char message1[LOG_MESSAGE_LENGTH];
    time(&now);
    if (read(fd1[0], message1, LOG_MESSAGE_LENGTH) > 0) {
        fprintf(log_file, "%d %.24s %s\n", logcounter, ctime(&now), message1);
    } else {
        return 1;
    }
    logcounter++;
    return 0;
}

int create_log_process() {

    if (pipe(fd1) == -1) {
        //error
        return 1;
    }
    logcounter = 0;

    pid = fork();

    if (pid < 0) {
        //error
        return 1;
    }

    if (pid>0) { // parent
        close(fd1[0]);

    } else if (pid==0) { //child process
        close(fd1[1]);
        log_file = fopen(LOG_FILE_NAME, "a"); // append file if it exists

        if (log_file == NULL) {
            return 1;
        }
        // make log line buffered, so no logs get lost
        if (setvbuf(log_file, NULL, _IOLBF, 0) != 0) {
            return 1;
        }

    }
    return 0;
}

int end_log_process() {
    if (pid>0) {
        // parent
        close(fd1[0]);
        close(fd1[1]);
        wait(NULL);

    } else if (pid==0) {
        if (fclose(log_file) != 0) {
            printf("Error closing " LOG_FILE_NAME ", bummer");
        }
        close(fd1[0]);
        close(fd1[1]);
        exit(0);
    }
    return 0;
}


