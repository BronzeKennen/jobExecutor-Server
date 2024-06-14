#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/worker.h"
#include "../include/controller.h"

extern int concurrency;

void* worker(void * arg) {
    printf("Fuck that we can go rizz for rizz\n");
    return NULL;
}
