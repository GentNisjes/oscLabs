// //
// // Created by stijn on 11/18/24.
// //
//
// #include <stdio.h>
// #include <string.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include<time.h>
// #include "logger.h"
//
// int fd1[2];
// pid_t pid;
// int logNumber;
// FILE* logFileName;
//
// //CHILD
// /* the child is the logger that will be activated with the create_log_process
//  * 1) needs to open the file to write in
//  * 2) close the writing file descriptor pipe_fd[1]
//  * 3) fprint to the file if message is no longer then the maximum input characters
//  * 4) fclose the file
//  * 5) close the reading file descriptor pipe_fd[0]
//
// */
//
//
// int write_to_log_process(char *msg) {
//
//     const size_t MSG_SIZE = 100;  // Fixed size for messages
//     char buffer[MSG_SIZE]; // Initialize a zeroed-out buffer
//
//     if (pid > 0) {
//         // Parent process
//         // Copy the message into the fixed-size buffer
//         strncpy(buffer, msg, MSG_SIZE - 1);  // Leave space for null terminator
//         buffer[MSG_SIZE - 1] = '\0';         // Ensure null-termination
//         write(fd1[1], buffer, MSG_SIZE);     // Write exactly MSG_SIZE bytes
//     }//  else if (pid == 0) {
//     //     // Child process
//     //     char logMessage[MSG_SIZE];  // Fixed-size buffer to read message
//     //
//     //     if (read(fd1[0], logMessage, MSG_SIZE) > 0) {
//     //         // Log the received message
//     //         time_t now = time(NULL);
//     //         logMessage[MSG_SIZE - 1] = '\0'; // Ensure null-termination
//     //         fprintf(logFileName, "%d - %.24s - %s\n", logNumber++, ctime(&now), logMessage);
//     //     }
//     // }
//
//     return 0;
// }
//
// int create_log_process() {
//     if (pipe(fd1) == -1) {
//         // Error
//         return 1;
//     }
//
//     // Reset the log number, since we're creating a new log process
//     logNumber = 0;
//
//     // Create a child process
//     pid = fork();
//
//     // Check if fork went ok
//     if (pid == -1) {
//         // Error
//         return 1;
//     }
//
//     if (pid > 0) {
//         // Parent
//         close(fd1[0]);
//     }else if (pid == 0) {
//         // Child process
//         close(fd1[1]); // Close the unused write-end of the pipe
//         logFileName = fopen("gateway.log", "a");
//
//         if (logFileName == NULL) {
//             perror("Failed to open log file");
//             exit(EXIT_FAILURE);
//         }
//
//         char logMessage[100];
//         while (read(fd1[0], logMessage, sizeof(logMessage)) > 0) {
//             time_t now = time(NULL);
//             logMessage[sizeof(logMessage) - 1] = '\0'; // Null-terminate the string
//             fprintf(logFileName, "%d - %.24s - %s\n", logNumber++, ctime(&now), logMessage);
//             fflush(logFileName); // Ensure the message is written immediately
//         }
//
//         // Clean up resources after reading
//         fclose(logFileName);
//         close(fd1[0]);
//         exit(0); // Exit the child process
//     }
//     return 0;
// }
//
// int end_log_process() {
//     if (pid > 0) {
//         close(fd1[0]);
//         close(fd1[1]);  // Close the write end of the pipe
//         wait(NULL);     // Wait for the child to exit
//     } else if (pid == 0) {
//         // Child process
//         fclose(logFileName);
//         close(fd1[0]);
//         close(fd1[1]);
//         exit(0);
//     }
//     return 0;
// }
//
//
// // if (pid > 0) {
// //     // parent
// //     // size_t msg_len = strlen(msg) + 1;  // +1 for null terminator
// //     // write(fd1[1], msg, msg_len);       // Send message with correct length
// //     write(fd1[1], msg, 25);
// // } else if (pid == 0) {
// //     // child
// //     // time_t now;
// //     // char message1[256];  // Make sure the buffer is large enough to store the message
// //     // ssize_t bytesRead = read(fd1[0], message1, sizeof(message1) - 1);
// //     //
// //     // if (bytesRead > 0) {
// //     //     message1[bytesRead] = '\0';  // Null-terminate the string
// //     //     time(&now);
// //     //     fprintf(logFileName, "%d - %.24s - %s\n", logNumber++, ctime(&now), message1);
// //     // }
// //     time_t now;
// //     char message1[25];
// //     time(&now);
// //     if (read(fd1[0], message1, 25) > 0) {
// //         fprintf(logFileName, "%d - %.24s - %s\n", logNumber, ctime(&now), message1);
// //     }
// //     logNumber++;
// // }
// //
// // return 0;


//
// Created by student on 11/18/24.
//
#include "logger.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/prctl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

int pipe_fd[2];

int child_pid = -10;
FILE* fptr = NULL;
char* log_buf;
int sequence_number = 0;
bool ending = false;
bool parent_is_dead = false;
time_t parent_death_time;
void SIGUSR1_log_handler(int i)
{
    parent_is_dead = true;
    parent_death_time = time(NULL);
}

void log_process_entry()
{
    int r = prctl(PR_SET_PDEATHSIG, SIGUSR1);
    if (r == -1) { perror(0); exit(1); }
    signal(SIGUSR1, SIGUSR1_log_handler);
    //When debugging the child process specifically, stopping on a breakpoint and then continuing will cause a SIGHUP.
    //This will then be caught in the debugger, and if you continue the program exits with a -1 status code.
    //These 2 lines prevent that.
    signal(SIGHUP, SIG_IGN);
    setsid();

    fptr = fopen("gateway.log", "a");
    log_buf = malloc(sizeof(char)*100);

    while (1)
    {
        ssize_t amount_read = read(pipe_fd[0], log_buf, sizeof(char)*100);
        if (amount_read == 100)
        {
            time_t current_time = time(NULL);
            char* time_string = ctime(&current_time);
            time_string[strlen(time_string)-1] = '\0';
            if (strcmp(log_buf, "ENDEND") == 0)
            {
                ending = true;
                goto checkend;
            }
            fprintf(fptr, "%d - %s - %s\n", sequence_number, time_string, log_buf);
            fflush(fptr);
            sequence_number++;
            continue;
        } else if (amount_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            sleep(1);
            goto checkend;
        } else if (amount_read < 100 && amount_read > 0) {
            printf("reading smaller amount then 100");
            exit(1);
        } else
        {
            printf("read error: %d\n", errno);
        }
    checkend:
        if (parent_is_dead && (time(NULL) - parent_death_time) > 10)
        {
            ending = true;
        }
        if (ending)
        {
            close(pipe_fd[0]);
            break;
        }
    }
    free(log_buf);
}

/**
 *
 * protocol: send a buffer of 100 bytes through every time. Pad the unused part of the buffer with zeroes.
 */
int write_to_log_process(char* msg)
{
    char* tmp_buf = malloc(sizeof(char)*100);
    memset(tmp_buf, 0, sizeof(char)*100);
    strcpy(tmp_buf, msg);
    int result = write(pipe_fd[1], (void*) tmp_buf, sizeof(char)*100); //this conversion should be fine since the number will be max 100(size of the buffer) and min -1 (error).
    free(tmp_buf);
    if (result < 0)
    {
        printf("Could not write the following to log process: %s", msg);
        return result;
    }
    if (result < 100)
    {
        printf("less bytes written");
        return -1;
    }
    return 0;
}

int create_log_process()
{

    int pipe_result = pipe(pipe_fd);
    if (pipe_result < 0)
    {
        printf("Could not create pipe");
        return -1;
    }
    int fcntl_result = fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);
    if (fcntl_result < 0)
    {
        printf("Could not create pipe");
        return -1;
    }
    pid_t ppid_before_fork = getpid();
    pid_t pid = fork();
    if (pid > 0) //parent
    {
        return 0;
    }
    log_process_entry();
    exit(0);
}

//The parent process can safely exit after this. Either the child process exits first and there is no orphan, or the parent process exits first, the child gets a SIGUSR1, waits 10 seconds to handle anymore data and exits.
//No zombies and no need for a wait() call since the parent itself exits at that moment.
//No orphans, No zombies, No prisoners. All dead.
int end_log_process() {
    ending = true;
    close(pipe_fd[1]);
    write_to_log_process("ENDEND");
    return 0;
}

