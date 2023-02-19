//
// Created by andreas on 19.02.23.
//

#include <pthread.h>
#include "errors.h"

#define ITERATIONS 10

pthread_mutex_t mutex_array[3] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

int backoff = 1; /* Whether to backoff or deadlock */
int yield_flag = 0; /* 0: no yield, >0: yield, <0: sleep */

void *lock_forward(void *arguments)
{
	int i{}, iterate{}, backoffs{};
	int status{};
	for (iterate = 0; iterate < ITERATIONS; ++iterate) {
		backoffs = 0;
		for (i = 0; i < 3; ++i) {
			if (i == 0)
				status = pthread_mutex_lock(&mutex_array[i]);
			if (status != 0)
				err_abort(status, "Error in lock_forward, locking first mutex");
			else {
				if (backoff)
					status = pthread_mutex_trylock(&mutex_array[i]);
				else
					status = pthread_mutex_lock(&mutex_array[i]);
				if (status == EBUSY) {
					backoffs++;
					DPRINTF((" [forward locker backing off at mutex %d ]\n", i));
					for(; i>=0; i--)
					{
						status = pthread_mutex_unlock(&mutex_array[i]);
						if (status != 0)
							err_abort(status, "Error during BACKOFF!")
					}
				}

			}

		}
	}
}