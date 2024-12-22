#ifndef CONNMGR_H
#define CONNMGR_H

#include "sbuffer.h"
#include "lib/tcpsock.h"

// Structure for passing arguments to the accept_connections thread
typedef struct accept_thread_args {
    tcpsock_t *server_socket;  // Pointer to the server socket
    int *conn_counter;         // Pointer to the connection counter
    int max_conn;              // Maximum number of connections
    sbuffer_t *buffer;         // Shared buffer
} accept_thread_args_t;

// Structure for passing arguments to the handle_client thread
typedef struct client_thread_args {
    tcpsock_t *client_socket;  // Pointer to the client socket
    sbuffer_t *buffer;         // Shared buffer
    int *conn_counter;         // Pointer to the connection counter
} client_thread_args_t;

// Function to handle individual client connections
void *handle_client(void *args);

// Function to accept incoming client connections
void *accept_connections(void *args);

// Connection manager function to initialize the server, start accepting connections, and handle clients
void connmgr(int port, int max_conn, sbuffer_t *buffer);

#endif // CONNMGR_H


// Created by douwe on 16/12/23.
//

// #ifndef CONNMGR_H
// #define CONNMGR_H
//
// #include "sbuffer.h"
// #include "lib/tcpsock.h"
//
// // Struct to pass connection manager arguments
// typedef struct {
//     tcpsock_t *client;
//     sbuffer_t *buffer;
// } conn_args_t;
//
// // Function to start the connection manager
// // port: The port number to listen on
// // max_conn: Maximum number of client connections
// // sbuffer: Shared buffer to store sensor data
// int connmgr(int port, int max_conn, sbuffer_t *sbuffer);
//
// // Thread function for handling individual client connections
// // Takes a `conn_args_t` struct as an argument
// int connection(void *connection_args);
//
// #endif // CONNMGR_H




