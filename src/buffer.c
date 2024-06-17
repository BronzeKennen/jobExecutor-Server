#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/buffer.h"

int id = 0;

void bufferAdd(shared_buffer_t * buf, job* job,int size) {
    pthread_mutex_lock(&buf->mutex);
    job->id = id++;
    while(buf->count == size) {
        pthread_cond_wait(&buf->not_full,&buf->mutex);
    }
    buf->buffer[buf->in] = job;
    buf->in = (buf->in +1) % size;
    buf->count++;
    pthread_cond_signal(&buf->not_empty);
    pthread_mutex_unlock(&buf->mutex); 

}
//A worker thread should check if it read something not null
job* bufferRemove(shared_buffer_t* buf,int size) {
    pthread_mutex_lock(&buf->mutex);
    while(buf->count == 0) {
        pthread_cond_wait(&buf->not_empty,&buf->mutex);
    }
    job* job = buf->buffer[buf->out];
    int i = 0;
    while(!job && i < size) { //Should also check if buffer got emptied with job stop
        buf->out = (buf->out + 1) % size;
        job = buf->buffer[buf->out];
        i++;
    }
    if(!job) return NULL;
    buf->count--;
    buf->buffer[buf->out] = NULL;
    pthread_cond_signal(&buf->not_full);
    pthread_mutex_unlock(&buf->mutex);
    return job;

}

job* bufferRemoveOnFind(shared_buffer_t* buf,job* toFind,int size) {
    job* job;
    if(!toFind) return NULL;
    pthread_mutex_lock(&buf->mutex);
    for(int i = 0; i < size; i++) {
        job = buf->buffer[i];
        if(!job) continue;
        if(job->id == toFind->id) {
            buf->buffer[i] = NULL;
            buf->count--;
            pthread_mutex_unlock(&buf->mutex);
            return job;            
        }

    }
    pthread_mutex_unlock(&buf->mutex);
    return NULL;

}

void bufferPrint(shared_buffer_t* buf,int size) { 
    int printed = 0;
    for(int i = 0; i < size; i++) {
        job* job = buf->buffer[i];
        if(!job) continue;
        printf("< %d, %d, %s >\n",job->id,job->socketFd,job->job);
        printed++;

    }
    if(!printed) printf("No jobs to print.\n");
}

