#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"


/**
 * Implements a sequential test server (only one connection at the same time)
 */

typedef struct accept_thread_args {
    tcpsock_t *server_socket; // Pointer to the server socket
    int *conn_counter;        // Pointer to the connection counter
    int max_conn;             // Maximum number of connections
} accept_thread_args_t;

void *handle_client(void *client_ptr) {
    tcpsock_t *client = (tcpsock_t *)client_ptr;
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

        // Print the received data
        printf("Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n",
               data.id, data.value, (long)data.ts);
    }

    if (result == TCP_CONNECTION_CLOSED) {
        printf("Client disconnected\n");
    } else {
        printf("Error occurred with client connection\n");
    }

    tcp_close(&client);
    return NULL;
}

void *accept_connections(void *args) {
    struct accept_thread_args *thread_args = (struct accept_thread_args *)args;
    tcpsock_t *server = thread_args->server_socket;
    int *conn_counter = thread_args->conn_counter;
    int max_conn = thread_args->max_conn;

    while (*conn_counter <= max_conn) {
        tcpsock_t *client;
        //wait for a possible client
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            fprintf(stderr, "Error while waiting for connection.\n");
            continue;
        }

        printf("Incoming client connection\n");
        (*conn_counter)++;

        // Spawn a new thread to handle the client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)client) != 0) {
            fprintf(stderr, "Failed to create client thread.\n");
            tcp_close(&client); // Close the client socket if thread creation fails
            (*conn_counter)--;  // Decrement counter since client couldn't be handled
            continue;
        }

        // Detach thread to let it clean up its resources after finishing
        pthread_detach(client_thread);
    }

    return NULL;
}



int main(int argc, char *argv[]) {
    tcpsock_t *server,     //listening socket, which accepts incoming connections.
              *client;     //temporarily represent each connected client socket during the handling process.
    sensor_data_t data;
    int bytes, result;     //track the number of bytes read or written over a socket.
    int conn_counter = 0;  //counter for the amount of client connections being handled
                           //see MAX_CONN
    pthread_t accept_thread;
    
    if(argc < 3) {         //program expects 2 command line arguments
    	printf("Please provide the right arguments: first the port, then the max nb of clients");
    	return -1;
    }
    
    int MAX_CONN = atoi(argv[2]);    //Extract the max amount of connections
    int PORT = atoi(argv[1]);        //the port this program should listen for incoming data

    printf("Test server is started\n");

    // Initialize the server-side socket for listening on the specified PORT
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to initialize the server socket.\n");
        exit(EXIT_FAILURE);
    }

    // Shared data for the accept thread
    accept_thread_args_t thread_args = {server, &conn_counter, MAX_CONN};


    // Start the thread to accept incoming connections
    if (pthread_create(&accept_thread, NULL, &accept_connections, (void *)&thread_args) != 0) {
        fprintf(stderr, "Failed to create accept thread.\n");
        exit(EXIT_FAILURE);
    }

    // Wait for the accept thread to complete (i.e., all connections processed)
    pthread_join(accept_thread, NULL);

    // Close the server socket after all clients are handled
    if (tcp_close(&server) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to close server socket.\n");
        exit(EXIT_FAILURE);
    }

    printf("Test server is shutting down\n");
    return 0;
}












