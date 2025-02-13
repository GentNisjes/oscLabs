//
// Created by stijn on 12/14/24.
//

#include <stdlib.h>
#include <pthread.h>
#include "config.h"
#include "sbuffer.h"

#include <stdio.h>

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    int stage;                  /**< indicator of how many times the node has been read = what stage of the process it is on */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

pthread_mutex_t buffermutex;
pthread_cond_t filled, updated;     //condition variable from the pthread lib
 									//to handle situations where the shared buffer is empty.
 									//-> pthread_cond_wait and
 									//-> pthread_cond_signal

// initialize mutex
// initialize stateBuff to be used for wait and signal function calls
int sbuffer_init(sbuffer_t **buffer) {
    pthread_mutex_init(&buffermutex, NULL);
    pthread_cond_init(&filled, NULL);
    pthread_cond_init(&updated, NULL);
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&buffermutex);
    if ((buffer == NULL) || (*buffer == NULL)) {
        pthread_mutex_unlock(&buffermutex);
        return SBUFFER_FAILURE;
    }

    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        if ((*buffer)->head == (*buffer)->tail) // buffer has only one node
        {
            (*buffer)->head = (*buffer)->tail = NULL; // remove end marker (0)
        } else  // buffer has many nodes
        {
            (*buffer)->head = (*buffer)->head->next;
        }
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    pthread_mutex_unlock(&buffermutex);
    pthread_cond_destroy(&filled);
    pthread_mutex_destroy(&buffermutex);
    return SBUFFER_SUCCESS;
}


// //Consumer threads will use this function
// //we will be accessing the shared mem, so lock that shizzle
//
// /* so adding mutex stuff
//  * including a case where we wait until an empty buffer gets new data
//  *
//  * now compared to the milestone 3 sbuffer we add an extra parameter called the state
//  * this is needed since we have a strict accessing order for the buffer reading + removing
//  *
//  */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, int stage) {
    //char logmsg[LOG_MESSAGE_LENGTH];
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&buffermutex);
    if (buffer == NULL) {
        pthread_mutex_unlock(&buffermutex);
        return SBUFFER_FAILURE;
    }
    while (buffer->head == NULL || buffer->head->stage != stage-1) {
        // block if head is null or node is at the wrong stage
        // write_to_log_process("SBuff REMOVE: waiting 4ever\n");
        // if (buffer->head == NULL) {
        //     sprintf(logmsg, "SBuff REMOVE: The head is NULL, the current state: %i\n", stage);
        //     write_to_log_process(logmsg);
        // } else {
        //     sprintf(logmsg, "SBuff REMOVE: The head state: %i, the current state: %i\n", buffer->head->stage, stage);
        //     write_to_log_process(logmsg);
        // }
        pthread_cond_wait(&updated, &buffermutex);
    }
    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head->data.id == 0) //end marker
    {
        // don't remove end marker to make sure every thread sees it
    } else if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    } else  // buffer has many nodes
    {
        buffer->head = buffer->head->next;
    }
    free(dummy);
    pthread_mutex_unlock(&buffermutex);
    return SBUFFER_SUCCESS;
}

// used by the datamgr part
// read, not remove,  still increase the stage so that the sensor_db comes next
int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data, int stage) {
    //char logmsg[LOG_MESSAGE_LENGTH];
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&buffermutex);
    if (buffer == NULL) {
        pthread_mutex_unlock(&buffermutex);
        return SBUFFER_FAILURE;
    }
    while (buffer->head == NULL || buffer->tail->stage != stage-1) {
        // block if head is null or all nodes are at the wrong stage (if final node is at the wrong stage, all of them are)
        // write_to_log_process("SBuff REMOVE: waiting 4ever\n");
        // if (buffer->head == NULL) {
        //     sprintf(logmsg, "SBuff READ: The head is NULL, the current state: %i\n", stage);
        //     write_to_log_process(logmsg);
        // } else {
        //     sprintf(logmsg, "SBuff READ: The head state: %i, the current state: %i\n", buffer->head->stage, stage);
        //     write_to_log_process(logmsg);
        // }
        pthread_cond_wait(&filled, &buffermutex);
    }
    dummy = buffer->head;
    while (dummy->stage != stage-1) { // read nodes that aren't the first one if the first one already has a high enough stage, to speed things up
        if (dummy->next == NULL) {
            pthread_mutex_unlock(&buffermutex);
            return SBUFFER_FAILURE;
        }
        dummy = dummy->next;
    }
    dummy->stage++; // increment stage count
    *data = dummy->data;
    if (dummy->data.id == 0) //end of buffer marker
    {
        // do nothing
    }
    pthread_mutex_unlock(&buffermutex);
    pthread_cond_signal(&updated);
    return SBUFFER_SUCCESS;
}

//writer threads will use this function
//used by the connmgr, set state back to zero
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data, int stage) {
    //char logmsg[LOG_MESSAGE_LENGTH];
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&buffermutex);
    if (buffer == NULL) {
        pthread_mutex_unlock(&buffermutex);
        return SBUFFER_FAILURE;
    }
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) {
        pthread_mutex_unlock(&buffermutex);
        return SBUFFER_FAILURE;
    }
    dummy->data = *data;
    dummy->stage = stage;
    dummy->next = NULL;
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL)
    {
        buffer->head = buffer->tail = dummy;
        // sprintf(logmsg,"SBuff INSERT: buff empty next = dummy with state %i\n", buffer->head->stage);
        // write_to_log_process(logmsg);
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
        // sprintf(logmsg,"SBuff INSERT: buff not empty next = dummy with state %i\n", buffer->head->stage);
        // write_to_log_process(logmsg);
    }
    pthread_mutex_unlock(&buffermutex);
    pthread_cond_signal(&filled);
    return SBUFFER_SUCCESS;
}

int sbuffer_cond(int amount) {
    for (int i = 0; i<amount; i++) {
        pthread_cond_signal(&filled);
    }
    return SBUFFER_SUCCESS;
}





