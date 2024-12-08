/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"


pthread_mutex_t mutexBuff;
pthread_cond_t stateBuff;		//condition variable from the pthread lib
								//to handle situations where the shared buffer is empty.
								//-> pthread_cond_wait and
								//-> pthread_cond_signal
/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
};

// initialize mutex
// initialise stateBuff to be used for wait and signal function calls
int sbuffer_init(sbuffer_t **buffer) {
  	pthread_mutex_init(&mutexBuff, NULL);
    pthread_cond_init(&stateBuff, NULL);
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    return SBUFFER_SUCCESS;
}

// mutex lock and unlock
int sbuffer_free(sbuffer_t **buffer) {
  	//end of stream marker: dummy
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&mutexBuff);
    if ((buffer == NULL) || (*buffer == NULL)) {
        pthread_mutex_unlock(&mutexBuff);
        return SBUFFER_FAILURE;
    }
    //adjusted this loop to end when the tail == head == null
    while ((*buffer)->head) {
      	//dummy will keep the address of the node that needs to be removed
        //otherwise we cant free the location, bcuz we lost the address
        dummy = (*buffer)->head;
    	if ((*buffer)->head == (*buffer)->tail) {
        	(*buffer)->head = (*buffer)->tail = NULL;
    	} else {
        	(*buffer)->head = (*buffer)->head->next;
    	}
    	free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

//Consumer threads will use this function
//we will be accessing the shared mem, so lock that shizzle
/* so adding mutex stuff
 * including a case where we wait until an empty buffer gets new data
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&mutexBuff);
    if (buffer == NULL) {
        pthread_mutex_unlock(&mutexBuff);
        return SBUFFER_FAILURE;
    }
    //use cond variable stateBuff for waiting
    while (buffer->head == NULL) {
        pthread_cond_wait(&stateBuff, &mutexBuff);
    }
    *data = buffer->head->data;
    dummy = buffer->head;

    if (buffer->head->data.id == 0) { // End marker detected
        // Don't remove end marker to ensure other threads see it
        pthread_mutex_unlock(&mutexBuff);
        return SBUFFER_SUCCESS;
    } else if (buffer->head == buffer->tail) { // Only one node in buffer
        buffer->head = buffer->tail = NULL;
    } else { // More than one node in buffer
        buffer->head = buffer->head->next;
    }

    free(dummy);
    pthread_mutex_unlock(&mutexBuff);
    return SBUFFER_SUCCESS;
}

//writer threads will use this function
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    pthread_mutex_lock(&mutexBuff);
    if (buffer == NULL) {
        pthread_mutex_unlock(&mutexBuff);
        return SBUFFER_FAILURE;
    }
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) {
        pthread_mutex_unlock(&mutexBuff);
        return SBUFFER_FAILURE;
    }
    dummy->data = *data;
    dummy->next = NULL;
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    return SBUFFER_SUCCESS;
}
