#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "config.h"
#include "lib/tcpsock.h"


/**
 * Implements a sequential test server (only one connection at the same time)
 */

int main(int argc, char *argv[]) {
    tcpsock_t *server,     //listening socket, which accepts incoming connections.
              *client;     //temporarily represent each connected client socket during the handling process.
    sensor_data_t data;
    int bytes, result;     //track the number of bytes read or written over a socket.
    int conn_counter = 0;  //counter for the amount of client connections being handled
                           //see MAX_CONN
    
    if(argc < 3) {         //program expects 2 command line arguments
    	printf("Please provide the right arguments: first the port, then the max nb of clients");
    	return -1;
    }
    
    int MAX_CONN = atoi(argv[2]);    //Extract the max amount of connections
    int PORT = atoi(argv[1]);        //the port this program should listen for incoming data

    printf("Test server is started\n");

    //Initializes the server-side socket for listening on the specified PORT.
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    //main loop to handle the connections until MAX_CONN is reached
    do {
        //tcp_wait_for_connection blocks until a client connects to the server.
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        conn_counter++;

        //
        do {
            //initialise the connection manager thread


            //READ out the data
            // read sensor ID
            bytes = sizeof(data.id);
            result = tcp_receive(client, (void *) &data.id, &bytes);
            // read temperature
            bytes = sizeof(data.value);
            result = tcp_receive(client, (void *) &data.value, &bytes);
            // read timestamp
            bytes = sizeof(data.ts);
            result = tcp_receive(client, (void *) &data.ts, &bytes);

            //check if the tcp connection completes and if bytes is not empty...
            if ((result == TCP_NO_ERROR) && bytes) {
                printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                       (long int) data.ts);
            }

        //continue while tcp connection is up and running
        } while (result == TCP_NO_ERROR);
        if (result == TCP_CONNECTION_CLOSED)
            printf("Peer has closed connection\n");
        else
            printf("Error occured on connection to peer\n");
        tcp_close(&client);
    } while (conn_counter < MAX_CONN);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    return 0;
}

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

void *accept_connections(void *server_ptr) {
    tcpsock_t *server = (tcpsock_t *)server_ptr;
    tcpsock_t *client;

    while (1) {
        if (tcp_wait_for_connection(server, &client) == TCP_NO_ERROR) {
            printf("New client connected\n");

            // Spawn a new thread to handle the client
            pthread_t client_thread;
            //important to give th address of the function, so &handle_client,
            //and declare handle_client() before accept connections
            if (pthread_create(&client_thread, NULL, &handle_client, (void *)client) != 0) {
                perror("Failed to create client thread");
                tcp_close(&client);
            } else {
                // Detach the thread so it cleans up resources on its own
                pthread_detach(client_thread);
            }
        }
    }
    return NULL;
}








