#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/buffer.h"

void bufferAdd(shared_buffer_t * buf, job* job,int size) {
    pthread_mutex_lock(&buf->mutex);
    while(buf->count == size) {
        pthread_cond_wait(&buf->not_full,&buf->mutex);
    }
    buf->buffer[buf->in] = job;
    buf->in = (buf->in +1) % size;
    buf->count++;
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->mutex); 

}

job* bufferRemove(shared_buffer_t* buf,int size) {
    pthread_mutex_lock(&buf->mutex);
    while(buf->count == 0) {
        pthread_cond_wait(&buf->not_empty,&buf->mutex);
    }
    job* job = buf->buffer[buf->out];
    buf->out = (buf->out + 1) % size;
    buf->count--;
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
    printf("double pump with the fanum tax\n");
    buf->buffer[buf->out-1] = NULL;
    return job;

}

void bufferPrint(shared_buffer_t* buf) { //Doesn't work as intended.  Need to initialize buffer
    for(int i = 0; i < buf->count; i++) {
        job* job = buf->buffer[i];
        printf("< %d, %d, %s >\n",job->id,job->socketFd,job->job);

    }
}

