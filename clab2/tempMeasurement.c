//
// Created by stijn on 10/21/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define frequency 1
#define MIN -10.0
#define MAX 35.0

float generateRandTemp () {
    return MIN + ((float)rand() / RAND_MAX) * (MAX - MIN);
}

int main () {
    srand(time(NULL));
    while(1) {
        float temp = generateRandTemp();

        // Get the current time
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        printf("Temperature = %1.2f @%s", temp, asctime(timeinfo));
        fflush(stdout);

        sleep(frequency);
    }
    return 0;
}
