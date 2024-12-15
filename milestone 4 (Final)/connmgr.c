//
// Created by stijn on 12/14/24.
//

#include "connmgr.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"  // For shared buffer operations

/**
 * Implements a concurrent test server with shared buffer insertion and logging
 */

// Structure for passing arguments to the accept_connections thread
typedef struct accept_thread_args {
    tcpsock_t *server_socket; // Pointer to the server socket
    int *conn_counter;        // Pointer to the connection counter
    int max_conn;             // Maximum number of connections
    sbuffer_t *buffer;        // Shared buffer
} accept_thread_args_t;

// Structure for passing arguments to the handle_client thread
typedef struct client_thread_args {
    tcpsock_t *client_socket; // Pointer to the client socket
    sbuffer_t *buffer;        // Shared buffer
    int *conn_counter;        // Pointer to the connection counter
} client_thread_args_t;

// Function to handle individual client connections
void *handle_client(void *args) {
    client_thread_args_t *client_args = (client_thread_args_t *)args;
    tcpsock_t *client = client_args->client_socket;
    sbuffer_t *buffer = client_args->buffer;
    int *conn_counter = client_args->conn_counter;
    sensor_data_t data;
    int bytes, result;

    while (1) {
        // Read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Insert data into the shared buffer
        if (sbuffer_insert(buffer, &data, 0) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into buffer.\n");
            break;
        }

        printf("Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n",
               data.id, data.value, (long)data.ts);
    }

    if (result == TCP_CONNECTION_CLOSED) {
        printf("Client disconnected\n");
    } else {
        printf("Error occurred with client connection\n");
    }

    // Clean up
    tcp_close(&client);
    free(client_args);
    (*conn_counter)--;

    return NULL;
}

// Function to accept incoming client connections
void *accept_connections(void *args) {
    accept_thread_args_t *thread_args = (accept_thread_args_t *)args;
    tcpsock_t *server = thread_args->server_socket;
    int *conn_counter = thread_args->conn_counter;
    int max_conn = thread_args->max_conn;
    sbuffer_t *buffer = thread_args->buffer;

    while (*conn_counter < max_conn) {
        tcpsock_t *client;

        // Wait for a possible client
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error while waiting for connection.\n");
            continue;
        }

        printf("Incoming client connection\n");
        (*conn_counter)++;

        // Prepare arguments for the client thread
        client_thread_args_t *client_args = malloc(sizeof(client_thread_args_t));
        if (client_args == NULL) {
            fprintf(stderr, "Failed to allocate memory for client arguments.\n");
            tcp_close(&client);
            (*conn_counter)--;
            continue;
        }

        client_args->client_socket = client;
        client_args->buffer = buffer;
        client_args->conn_counter = conn_counter;

        // Spawn a new thread to handle the client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)client_args) != 0) {
            fprintf(stderr, "Failed to create client thread.\n");
            tcp_close(&client);
            free(client_args);
            (*conn_counter)--;
            continue;
        }

        // Detach the thread to let it clean up its resources after finishing
        pthread_detach(client_thread);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    tcpsock_t *server;       // Listening socket for incoming connections
    int conn_counter = 0;    // Counter for active client connections
    pthread_t accept_thread; // Thread for accepting connections
    sbuffer_t *buffer;       // Shared buffer for storing sensor data

    if (argc < 3) {
        printf("Please provide the right arguments: first the port, then the max number of clients.\n");
        return -1;
    }

    int MAX_CONN = atoi(argv[2]);  // Extract the maximum number of connections
    int PORT = atoi(argv[1]);      // The port to listen for incoming data

    printf("Test server is started on port %d\n", PORT);

    // Initialize the shared buffer
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "Failed to initialize the shared buffer.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the server-side socket for listening on the specified PORT
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to initialize the server socket.\n");
        sbuffer_free(&buffer);
        exit(EXIT_FAILURE);
    }

    // Prepare arguments for the accept thread
    accept_thread_args_t thread_args = {server, &conn_counter, MAX_CONN, buffer};

    // Start the thread to accept incoming connections
    if (pthread_create(&accept_thread, NULL, accept_connections, (void *)&thread_args) != 0) {
        fprintf(stderr, "Failed to create accept thread.\n");
        tcp_close(&server);
        sbuffer_free(&buffer);
        exit(EXIT_FAILURE);
    }

    // Wait for the accept thread to complete (i.e., all connections processed)
    pthread_join(accept_thread, NULL);

    // Close the server socket
    if (tcp_close(&server) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to close server socket.\n");
    }

    // Free the shared buffer
    sbuffer_free(&buffer);

    printf("Test server is shutting down\n");
    return 0;
}
