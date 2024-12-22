// #include "connmgr.h"
// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include "lib/tcpsock.h"
// #include "sbuffer.h"
// #include "config.h"
// #include "inttypes.h" 	//for the PRIu16 type
//
// //// Structure for passing arguments to the accept_connections thread
// //typedef struct accept_thread_args {
// //    tcpsock_t *server_socket; // Pointer to the server socket
// //    int *conn_counter;        // Pointer to the connection counter
// //    int max_conn;             // Maximum number of connections
// //    sbuffer_t *buffer;        // Shared buffer
// //} accept_thread_args_t;
// //
// //// Structure for passing arguments to the handle_client thread
// //typedef struct client_thread_args {
// //    tcpsock_t *client_socket; // Pointer to the client socket
// //    sbuffer_t *buffer;        // Shared buffer
// //    int *conn_counter;        // Pointer to the connection counter
// //} client_thread_args_t;
//
// // Function to handle individual client connections
// void *handle_client(void *args) {
//     client_thread_args_t *client_args = (client_thread_args_t *)args;
//     tcpsock_t *client = client_args->client_socket;
//     sbuffer_t *buffer = client_args->buffer;
//     int *conn_counter = client_args->conn_counter;
//     sensor_data_t data;
//     int bytes, result;
//
//     while (1) {
//         // Receive sensor data
//         bytes = sizeof(data.id);
//         result = tcp_receive(client, (void *)&data.id, &bytes);
//         if (result != TCP_NO_ERROR || bytes == 0) break;
//
//         bytes = sizeof(data.value);
//         result = tcp_receive(client, (void *)&data.value, &bytes);
//         if (result != TCP_NO_ERROR || bytes == 0) break;
//
//         bytes = sizeof(data.ts);
//         result = tcp_receive(client, (void *)&data.ts, &bytes);
//         if (result != TCP_NO_ERROR || bytes == 0) break;
//
//         // Insert data into the shared buffer
//         if (sbuffer_insert(buffer, &data, 0) != SBUFFER_SUCCESS) {
//             fprintf(stderr, "Failed to insert data into buffer.\n");
//         } else {
//             //printf("[%s]              Conn MGR: Data inserted into buffer - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n", get_timestamp(), data.id, data.value, (long)data.ts);
//             //fflush(stderr);
//             char log1[256];  // Adjust the size as needed
//             snprintf(log1, sizeof(log1),
//                      "Conn MGR: Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n",
//                      data.id, data.value, (long)data.ts);
//
//             write_to_log_process(log1);
//
//         }
//
//         //printf("[%s]              Conn MGR: Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n", get_timestamp(), data.id, data.value, (long)data.ts);
//         //fflush(stderr);
//     }
//
//     // Clean up
//     tcp_close(&client);
//     free(client_args);
//     (*conn_counter)--;
//
//     return NULL;
// }
//
// // Function to accept incoming client connections
// void *accept_connections(void *args) {
//     accept_thread_args_t *thread_args = (accept_thread_args_t *)args;
//     tcpsock_t *server = thread_args->server_socket;
//     int *conn_counter = thread_args->conn_counter;
//     int max_conn = thread_args->max_conn;
//     sbuffer_t *buffer = thread_args->buffer;
//
//     while (*conn_counter < max_conn) {
//         tcpsock_t *client;
//         if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
//             fprintf(stderr, "Error while waiting for connection.\n");
//             continue;
//         }
//
//         printf("Incoming client connection\n");
//         write_to_log_process("incoming client connection\n");
//         (*conn_counter)++;
//
//         // Prepare arguments for the client thread
//         client_thread_args_t *client_args = malloc(sizeof(client_thread_args_t));
//         if (client_args == NULL) {
//             fprintf(stderr, "Failed to allocate memory for client arguments.\n");
//             write_to_log_process("Failed to allocate memory for client arguments.\n");
//             tcp_close(&client);
//             (*conn_counter)--;
//             continue;
//         }
//
//         client_args->client_socket = client;
//         client_args->buffer = buffer;
//         client_args->conn_counter = conn_counter;
//
//         pthread_t client_thread;
//         if (pthread_create(&client_thread, NULL, handle_client, (void *)client_args) != 0) {
//             fprintf(stderr, "Failed to create client thread.\n");
//             write_to_log_process("Failed to create client thread.\n");
//             tcp_close(&client);
//             free(client_args);
//             (*conn_counter)--;
//             continue;
//         }
//
//         pthread_detach(client_thread);
//     }
//
//     return NULL;
// }
//
// // Connection manager function
// void connmgr(int port, int max_conn, sbuffer_t *buffer) {
//     tcpsock_t *server;
//     int conn_counter = 0;
//     pthread_t accept_thread;
//
//     printf("Conn MGR: Starting connection manager on port %d\n", port);
//     char log2[256];  // Adjust the size as needed
//     snprintf(log2, sizeof(log2),
//              "Conn MGR: Starting connection manager on port %d\n",
//              port);
//
//     write_to_log_process(log2);
//
//     if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
//         fprintf(stderr, "Failed to initialize the server socket.\n");
//         write_to_log_process("Failed to initialize the server socket.\n");
//         return;
//     }
//
//     accept_thread_args_t thread_args = {server, &conn_counter, max_conn, buffer};
//
//     if (pthread_create(&accept_thread, NULL, accept_connections, (void *)&thread_args) != 0) {
//         fprintf(stderr, "Failed to create accept thread.\n");
//         write_to_log_process("Failed to create accept thread.\n");
//         tcp_close(&server);
//         return;
//     }
//
//     pthread_join(accept_thread, NULL);
//
//     if (tcp_close(&server) != TCP_NO_ERROR) {
//         fprintf(stderr, "Failed to close server socket.\n");
//         write_to_log_process("Failed to close server socket.\n");
//     }
//
//     printf("Connection manager shutting down\n");
//     write_to_log_process("Connection manager shutting down\n");
// }

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"
#include "connmgr.h"

#include <inttypes.h>

#include "sensor_db.h"

int connmgr(int port, int max_conn, sbuffer_t *sbuffer) {
    tcpsock_t *server;

    pthread_t thread_id[max_conn];
    conn_args_t *cl_args[max_conn];

    for (int i = 0; i < max_conn; i++) {
        cl_args[i] = malloc(sizeof(conn_args_t));
        if (cl_args[i] == NULL) {
            write_to_log_process("Error allocating memory to argument struct, shutting down connection manager");
            return 1;
        }
        cl_args[i]->buffer = sbuffer;
    }
    write_to_log_process("Server started");

    // Start listening for client connections
    if (tcp_passive_open(&server, port) != TCP_NO_ERROR) {
        exit(EXIT_FAILURE);
    }

    for (int conn_counter = 0; conn_counter < max_conn; conn_counter++) {
        // Wait for connection (blocks until connection is found)
        if (tcp_wait_for_connection(server, &cl_args[conn_counter]->client) != TCP_NO_ERROR) {
            exit(EXIT_FAILURE);
        }

        // Create the thread
        if (pthread_create(&thread_id[conn_counter], NULL, (void *)connection, cl_args[conn_counter]) != 0) {
            return -1;
        }
    }

    for (int i = 0; i < max_conn; i++) { // Wait for every thread to end
        pthread_join(thread_id[i], NULL);
    }

    // Indicate end of sbuffer
    sensor_data_t data;
    data.id = 0;
    sbuffer_insert(sbuffer, &data, 0);

    if (tcp_close(&server) != TCP_NO_ERROR) {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < max_conn; i++) {
        free(cl_args[i]);
    }

    return 0;
}

int connection(void *connection_args) {
    conn_args_t *cl_args = (conn_args_t *)connection_args;
    int bytes, result;
    int id = 0;
    sensor_data_t data;
    char logmsg[LOG_MESSAGE_LENGTH];

    do {
        // Read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(cl_args->client, (void *)&data.id, &bytes);

        if (id == 0) {
            id = data.id; // Set persistent ID
        }
        if (result != TCP_NO_ERROR) {
            break;
        }

        // Read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(cl_args->client, (void *)&data.value, &bytes);

        if (result != TCP_NO_ERROR) {
            break;
        }

        // Read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(cl_args->client, (void *)&data.ts, &bytes);

        if ((result == TCP_NO_ERROR) && bytes) {
            if (sbuffer_insert(cl_args->buffer, &data, 0) == SBUFFER_SUCCESS) {
                char log1[256];  // Adjust the size as needed
                snprintf(log1, sizeof(log1),
                         "Conn MGR: Received data - Sensor ID: %" PRIu16 ", Temp: %g, Timestamp: %ld\n",
                         data.id, data.value, (long)data.ts);
                write_to_log_process(log1);
            }
        }
    } while (result == TCP_NO_ERROR);

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

    if (tcp_close(&cl_args->client) != TCP_NO_ERROR) {
        sprintf(logmsg, "Connection with sensor node %u not closed correctly", id);
        write_to_log_process(logmsg);
    }

    return 0;
}






