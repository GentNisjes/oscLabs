#include "connmgr.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include "config.h"
#include "inttypes.h" 	//for the PRIu16 type


// Function to handle individual client connections
void *handle_client(void *args) {
    client_thread_args_t *client_args = (client_thread_args_t *)args;
    tcpsock_t *client = client_args->client_socket;
    sbuffer_t *buffer = client_args->buffer;
    int *conn_counter = client_args->conn_counter;
    sensor_data_t data;
    char logmsg[LOG_MESSAGE_LENGTH];
    int bytes, result;
    int id = 0; //initial value of the id, when first reading a new connection sensor we dont know it
                //yet so set it to zero, for the log message we check on this value too.

    while (1) {
        // Receive sensor data
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes, TIMEOUT);

        if (result == TCP_CONNECTION_TIMEOUT) {
            snprintf(logmsg, sizeof(logmsg), "Sensor node %u timed out", id);
            write_to_log_process(logmsg);
            break;
        }

        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Log the first connection of the sensor node
        if (id == 0) { // Only on the first loop
            id = data.id;
            snprintf(logmsg, sizeof(logmsg), "Sensor node %u has opened a new connection", id);
            write_to_log_process(logmsg);
        }

        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes, TIMEOUT);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes, TIMEOUT);
        if (result != TCP_NO_ERROR || bytes == 0) break;

        // Insert data into the shared buffer
        if (sbuffer_insert(buffer, &data, 0) != SBUFFER_SUCCESS) {
            fprintf(stderr, "Failed to insert data into buffer.\n");
        } else {
            //printf("[%s]              Conn MGR: Data inserted into buffer - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n", get_timestamp(), data.id, data.value, (long)data.ts);
            //fflush(stderr);
            // char log1[256];  // Adjust the size as needed
            // snprintf(log1, sizeof(log1),
            //          "Conn MGR: Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n",
            //          data.id, data.value, (long)data.ts);
            //
            // write_to_log_process(log1);
        }
    }

    // write to log
    if (result == TCP_CONNECTION_CLOSED) {
        sprintf(logmsg, "Sensor node %u has closed the connection", id);
    }
    // else if (result == TCP_CONNECTION_TIMEOUT) {
    //     sprintf(logmsg, "Sensor node %u has timed out, connection closed", id);
    // }
    else {
        sprintf(logmsg, "Error connecting to sensor node %u, connection closed", id);
    }
    write_to_log_process(logmsg);

    // Clean up
    if (tcp_close(&client) != TCP_NO_ERROR) {
        sprintf(logmsg, "Connection with sensor node %u not closed correctly", id);
        write_to_log_process(logmsg);
    }
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
        write_to_log_process("incoming client connection");
        (*conn_counter)++;

        // Prepare arguments for the client thread
        client_thread_args_t *client_args = malloc(sizeof(client_thread_args_t));
        if (client_args == NULL) {
            fprintf(stderr, "Failed to allocate memory for client arguments.\n");
            write_to_log_process("Failed to allocate memory for client arguments.");
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
            write_to_log_process("Failed to create client thread.");
            tcp_close(&client);
            free(client_args);
            (*conn_counter)--;
            continue;
        }

        pthread_detach(client_thread);
        //pthread_join(client_thread, NULL);
    }

    while (*conn_counter > 0) {
        sleep(1); // Polling, an extra condition variable might have been more efficient not implemented due to a lack of time
    }

    // Indicate end of sbuffer when max connections are reached
    sensor_data_t end_signal = {0};
    sbuffer_insert(buffer, &end_signal, 0);

    return NULL;
}

// Connection manager function
void connmgr(int port, int max_conn, sbuffer_t *buffer) {
    tcpsock_t *server;
    int conn_counter = 0;
    pthread_t accept_thread;

    printf("Conn MGR: Starting connection manager on port %d\n", port);
    char log2[LOG_MESSAGE_LENGTH];
    snprintf(log2, sizeof(log2),
             "Conn MGR: Starting connection manager on port %d",
             port);

    write_to_log_process(log2);

    if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
        fprintf(stderr, "Failed to initialize the server socket.\n");
        write_to_log_process("Failed to initialize the server socket.\n");
        return;
    }
    write_to_log_process("Server started");

    accept_thread_args_t thread_args = {server, &conn_counter, max_conn, buffer};

    if (pthread_create(&accept_thread, NULL, accept_connections, (void *)&thread_args) != 0) {
        write_to_log_process("Failed to create accept thread.");
        tcp_close(&server);
        return;
    }

    pthread_join(accept_thread, NULL);

    if (tcp_close(&server) != TCP_NO_ERROR) {
        write_to_log_process("Failed to close server socket.");
    }

    write_to_log_process("All connections closed - connection manager finished");
}








