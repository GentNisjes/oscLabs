#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>

#include "connmgr.h"    // Your connection manager header file
#include "datamgr.h"
#include "sbuffer.h"    // Your sbuffer header file
#include "sensor_db.h" // Storage manager header file (assumed)

#define LOG_FILE_NAME  "gateway.log"

int fd1[2];
pid_t pid;
int logcounter;
FILE* log_file;

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
    sbuffer_t *sbuffer;
    if (sbuffer_init(&sbuffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize shared buffer.\n");
        exit(EXIT_FAILURE);
    }

    // pass arguments to manager threads
    connmgr_args_t *conn_args = malloc(sizeof *conn_args);
    datamgr_args_t *data_args= malloc(sizeof *data_args);
    storagemgr_args_t *storage_args= malloc(sizeof *storage_args);

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

    pthread_join(connmgr_thread, NULL);
    free(cm_args);
    conn_args = NULL;
    pthread_join(datamgr_id, NULL);
    free(data_args);
    data_args = NULL;
    pthread_join(storagemgr_id, NULL);
    free(storage_args);
    storage_args = NULL;

    // Cleanup: Free the shared buffer after both threads finish
    if (sbuffer_free(&sbuffer) != SBUFFER_SUCCESS) {
        write_to_log_process("Failed to free shared buffer.\n");
        exit(EXIT_FAILURE);
    }
    free(shared_data);
    shared_data = NULL;

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