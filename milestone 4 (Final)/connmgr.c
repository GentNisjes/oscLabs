#include "connmgr.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include "config.h"
#include "inttypes.h" 	//for the PRIu16 type

//// Structure for passing arguments to the accept_connections thread
//typedef struct accept_thread_args {
//    tcpsock_t *server_socket; // Pointer to the server socket
//    int *conn_counter;        // Pointer to the connection counter
//    int max_conn;             // Maximum number of connections
//    sbuffer_t *buffer;        // Shared buffer
//} accept_thread_args_t;
//
//// Structure for passing arguments to the handle_client thread
//typedef struct client_thread_args {
//    tcpsock_t *client_socket; // Pointer to the client socket
//    sbuffer_t *buffer;        // Shared buffer
//    int *conn_counter;        // Pointer to the connection counter
//} client_thread_args_t;

// Function to handle individual client connections
void *handle_client(void *args) {
    client_thread_args_t *client_args = (client_thread_args_t *)args;
    tcpsock_t *client = client_args->client_socket;
    sbuffer_t *buffer = client_args->buffer;
    int *conn_counter = client_args->conn_counter;
    sensor_data_t data;
    int bytes, result;

    while (1) {
        // Receive sensor data
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Insert data into the shared buffer
        if (sbuffer_insert(buffer, &data, 0) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into buffer.\n");
        } else {
            printf("Data inserted into buffer - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n", data.id, data.value, (long)data.ts);
            //fflush(stderr);
        }


        printf("Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n", data.id, data.value, (long)data.ts);
        //fflush(stderr);
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

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)client_args) != 0) {
            fprintf(stderr, "Failed to create client thread.\n");
            tcp_close(&client);
            free(client_args);
            (*conn_counter)--;
            continue;
        }

        pthread_detach(client_thread);
    }

    return NULL;
}

// Connection manager function
void connmgr(int port, int max_conn, sbuffer_t *buffer) {
    tcpsock_t *server;
    int conn_counter = 0;
    pthread_t accept_thread;

    printf("Starting connection manager on port %d\n", port);

    if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to initialize the server socket.\n");
        return;
    }

    accept_thread_args_t thread_args = {server, &conn_counter, max_conn, buffer};

    if (pthread_create(&accept_thread, NULL, accept_connections, (void *)&thread_args) != 0) {
        fprintf(stderr, "Failed to create accept thread.\n");
        tcp_close(&server);
        return;
    }

    pthread_join(accept_thread, NULL);

    if (tcp_close(&server) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to close server socket.\n");
    }

    printf("Connection manager shutting down\n");
}
