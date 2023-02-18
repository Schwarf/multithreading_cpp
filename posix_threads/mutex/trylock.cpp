//
// Created by andreas on 18.02.23.
//
#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter;
time_t end_time;

void *counter_thread(void *arg)
{
	int status;
	int spin;
}
