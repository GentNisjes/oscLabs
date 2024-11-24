//
// Created by stijn on 11/18/24.
//

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include<time.h>
#include "logger.h"

int fd1[2];
pid_t pid;
int logNumber;
FILE* logFileName;

//CHILD
/* the child is the logger that will be activated with the create_log_process
 * 1) needs to open the file to write in
 * 2) close the writing file descriptor pipe_fd[1]
 * 3) fprint to the file if message is no longer then the maximum input characters
 * 4) fclose the file
 * 5) close the reading file descriptor pipe_fd[0]

*/


int write_to_log_process(char *msg) {

    const size_t MSG_SIZE = 100;  // Fixed size for messages
    char buffer[MSG_SIZE]; // Initialize a zeroed-out buffer

    if (pid > 0) {
        // Parent process
        // Copy the message into the fixed-size buffer
        strncpy(buffer, msg, MSG_SIZE - 1);  // Leave space for null terminator
        buffer[MSG_SIZE - 1] = '\0';         // Ensure null-termination
        write(fd1[1], buffer, MSG_SIZE);     // Write exactly MSG_SIZE bytes
    }
    return 0;
}

int create_log_process() {
    if (pipe(fd1) == -1) {
        // Error
        return 1;
    }

    // Reset the log number, since we're creating a new log process
    logNumber = 0;

    // Create a child process
    pid = fork();

    // Check if fork went ok
    if (pid == -1) {
        // Error
        return 1;
    }

    if (pid > 0) {
        // Parent
        close(fd1[0]);
    }else if (pid == 0) {
        // Child process
        close(fd1[1]); // Close the unused write-end of the pipe
        logFileName = fopen("gateway.log", "a");

        if (logFileName == NULL) {
            perror("Failed to open log file");
            exit(EXIT_FAILURE);
        }

        char logMessage[100];
        while (read(fd1[0], logMessage, sizeof(logMessage)) > 0) {
            time_t now = time(NULL);
            logMessage[sizeof(logMessage) - 1] = '\0'; // Null-terminate the string
            fprintf(logFileName, "%d - %.24s - %s\n", logNumber++, ctime(&now), logMessage);
            fflush(logFileName); // Ensure the message is written immediately
        }

        // Clean up resources after reading
        fclose(logFileName);
        close(fd1[0]);
        exit(0); // Exit the child process
    }
    return 0;
}

int end_log_process() {
    if (pid > 0) {
        close(fd1[0]);
        close(fd1[1]);  // Close the write end of the pipe
        wait(NULL);     // Wait for the child to exit
    } else if (pid == 0) {
        // Child process
        fclose(logFileName);
        close(fd1[0]);
        close(fd1[1]);
        exit(0);
    }
    return 0;
}



