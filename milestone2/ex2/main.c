//
// Created by stijn on 11/18/24.
//

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main() {
    int pipefd[2]; // Array to hold the file descriptors for the pipe
    pid_t pid;
    char message[] = "Hi There";
    char buffer[BUFFER_SIZE];

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[1]); // Close the write end of the pipe (we are only reading)

        // Read the message from the pipe
        read(pipefd[0], buffer, BUFFER_SIZE);
        close(pipefd[0]); // Close the read end after reading

        // Reverse the case of each character
        for (int i = 0; i < strlen(buffer); i++) {
            if (islower(buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            } else if (isupper(buffer[i])) {
                buffer[i] = tolower(buffer[i]);
            }
        }

        // Print the modified message
        printf("Modified message: %s\n", buffer);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(pipefd[0]); // Close the read end of the pipe (we are only writing)

        // Write the message to the pipe
        write(pipefd[1], message, strlen(message) + 1);
        close(pipefd[1]); // Close the write end after writing

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}
