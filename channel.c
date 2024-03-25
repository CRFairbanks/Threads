
#include "channel.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
chan_t* channel_create(size_t size)
{
        /* IMPLEMENT THIS */
    if(size  > 0){
        /* Return buffered channel */
        // malloc
        // semaphores
        chan_t* channel = (chan_t *)(malloc(sizeof(chan_t)));
        channel->buffer = buffer_create(size);
        pthread_mutex_init(&channel->mutex, NULL);
        sem_init(&channel->semaphore_empty, 0, (unsigned)(size));
        sem_init(&channel->semaphore_full, 0, 0);
        channel->closed = 0;
        return channel;
    }
    if(size == 0){

    }
    return NULL;
}

// Writes data to the given channel
// This can be both a blocking call i.e., the function only returns on a successful completion of send (blocking = true), and
// a non-blocking call i.e., the function simply returns if the channel is full (blocking = false)
// In case of the blocking call when the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// WOULDBLOCK if the channel is full and the data was not added to the buffer (non-blocking calls only),
// CLOSED_ERROR if the channel is closed, and
// OTHER_ERROR on encountering any other generic error of any sort
enum chan_status channel_send(chan_t* channel, void* data, bool blocking)
{
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed == 0){
        pthread_mutex_unlock(&channel->mutex);
        if(blocking == true){
            sem_wait(&channel->semaphore_empty);
            pthread_mutex_lock(&channel->mutex);
            if(buffer_add(data, channel->buffer)){
                sem_post(&channel->semaphore_full);
                pthread_mutex_unlock(&channel->mutex);
                return SUCCESS;
            }
            sem_post(&channel->semaphore_empty);
            pthread_mutex_unlock(&channel->mutex);
            
             // check close -- NEEDED
            if(channel->closed == 1){
                return CLOSED_ERROR;
            }
            
            return OTHER_ERROR;
        }
        else if(blocking == false){
            if(sem_trywait(&channel->semaphore_empty) != 0){
                return WOULDBLOCK;
            }
            pthread_mutex_lock(&channel->mutex);
            if(buffer_add(data, channel->buffer)){
                sem_post(&channel->semaphore_full);
                pthread_mutex_unlock(&channel->mutex);
                return SUCCESS;
            }
            sem_post(&channel->semaphore_empty);
            pthread_mutex_unlock(&channel->mutex);
            return OTHER_ERROR;
        }
    }
    else{
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
        }
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This can be both a blocking call i.e., the function only returns on a successful completion of receive (blocking = true), and
// a non-blocking call i.e., the function simply returns if the channel is empty (blocking = false)
// In case of the blocking call when the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// WOULDBLOCK if the channel is empty and nothing was stored in data (non-blocking calls only),
// CLOSED_ERROR if the channel is closed, and
// OTHER_ERROR on encountering any other generic error of any sort
enum chan_status channel_receive(chan_t* channel, void** data, bool blocking)
{
    pthread_mutex_lock(&channel->mutex);
     if(channel->closed == 0){
        pthread_mutex_unlock(&channel->mutex);
        if(blocking == true){
            sem_wait(&channel->semaphore_full);
            pthread_mutex_lock(&channel->mutex);
            *data = buffer_remove(channel->buffer);
            if(*data != BUFFER_EMPTY){
                sem_post(&channel->semaphore_empty);
                pthread_mutex_unlock(&channel->mutex);
                 // check close
                if(channel->closed == 1){
                    return CLOSED_ERROR;
                }
                return SUCCESS;
            }
            sem_post(&channel->semaphore_full);
            pthread_mutex_unlock(&channel->mutex);
            // check close
            if(channel->closed == 1){
                return CLOSED_ERROR;
            }
            return OTHER_ERROR;
        }
        else if(blocking == false){
            if(sem_trywait(&channel->semaphore_full) == 0){
                // check close
                if(channel->closed == 1){
                    return CLOSED_ERROR;
                }
                return WOULDBLOCK;
            }
            pthread_mutex_lock(&channel->mutex);
            *data = buffer_remove(channel->buffer);
            if(*data != BUFFER_EMPTY){
                sem_post(&channel->semaphore_empty);
                pthread_mutex_unlock(&channel->mutex);
                // check close
                if(channel->closed == 1){
                    return CLOSED_ERROR;
                }
                return SUCCESS;
            }
            sem_post(&channel->semaphore_full);
            pthread_mutex_unlock(&channel->mutex);
             // check close
            if(channel->closed == 1){
                return CLOSED_ERROR;
            }
            return OTHER_ERROR;
        }
    else{
        pthread_mutex_unlock(&channel->mutex);;
        return CLOSED_ERROR;
        }
    }
    return SUCCESS;
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// OTHER_ERROR in any other error case
enum chan_status channel_close(chan_t* channel)
{
    // CHANNEL ALREADY CLOSED
    if(channel->closed == 1){
        return CLOSED_ERROR;
    }
    // ACCOUNTS FOR SEND/RECEIVE BECAUSE IF CHANNEL->CLOSED != 0 THEN THEY WILL BOTH REACH THE LAST ELSE
    // ** MAKE SURE TO ACCOUNT FOR THIS^ IN SELECT WHEN WE GET TO IT **
    else{
        pthread_mutex_lock(&channel->mutex);
        channel->closed = 1;
        sem_post(&channel->semaphore_empty);
        sem_post(&channel->semaphore_full);
        pthread_mutex_unlock(&channel->mutex);
    }
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// OTHER_ERROR in any other error case
enum chan_status channel_destroy(chan_t* channel)
{
    // OPEN CHANNEL
    if(channel->closed == 0){
        return DESTROY_ERROR;
    }
    buffer_free(channel->buffer);
    pthread_mutex_destroy(&channel->mutex);
    sem_destroy(&channel->semaphore_empty);
    sem_destroy(&channel->semaphore_full);
    free(channel);
    return SUCCESS;
}

// Takes an array of channels, channel_list, of type select_t and the array length, channel_count, as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum chan_status channel_select(size_t channel_count, select_t* channel_list, size_t* selected_index)
{
    /* IMPLEMENT THIS */
    return SUCCESS;
}
