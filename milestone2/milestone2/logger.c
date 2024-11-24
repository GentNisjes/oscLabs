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
 * 3) fprint to the file if message is not longer then the maximum input characters
 * 4) fclose the file
 * 5) close the reading file descriptor pipe_fd[0]
*/



int write_to_log_process(char *msg) {
    if (pid>0) {
        // parent
        write(fd1[1], msg, 25);

    } else if (pid==0) {
        char message1[25];
        time_t now;
        time(&now);     //store current time system and store in now

        //length of message check
        if (read(fd1[0], message1, 25) > 0) {
            fprintf(logFileName, "%d - %.24s - %s\n", logNumber, ctime(&now), message1);
        }
        fprintf(logFileName, "%d - %.24s - %s\n", logNumber, ctime(&now), message1);
        logNumber++;
    }

    return 0;
}

int create_log_process() {
    if (pipe(fd1) == -1) {
        //error
        return 1;
    }
    //reset the lognumber, bcs new log process
    logNumber = 0;

    //make a child process
    pid = fork();

    //check if fork went ok
    if (pid == -1) {
        //error
        return 1;
    }

    if (pid>0) {
        // parent
        close(fd1[0]);

    } else if (pid==0) {
        //child process
        close(fd1[1]);
        logFileName = fopen("gateway.log", "a");

        if (logFileName == NULL) {
            return 1;
        }
    }
}

int end_log_process() {
    if (pid>0) {
        // parent
        close(fd1[0]);
        close(fd1[1]);
        wait(NULL);

    } else if (pid==0) {
        //child
        fclose(logFileName);
        close(fd1[0]);
        close(fd1[1]);
        exit(0);
    }
    return 0;
}

