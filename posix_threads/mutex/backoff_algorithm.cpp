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
					for (; i >= 0; i--) {
						status = pthread_mutex_unlock(&mutex_array[i]);
						if (status != 0)
							err_abort(status, "Error during BACKOFF!");
					}
				}
				else {
					if (status != 0)
						err_abort(status, "Error while locking mutex");
					DPRINTF ((" forward locker got mutex %d locked\n", i));
				}
			}
			if (yield_flag) {
				if (yield_flag > 0)
					sched_yield();
				else
					sleep(1);
			}

		}
		printf("lock_forward got all locks, %d backoffs \n", backoffs);
		pthread_mutex_unlock(&mutex_array[2]);
		pthread_mutex_unlock(&mutex_array[1]);
		pthread_mutex_unlock(&mutex_array[0]);
		sched_yield();

	}
	return nullptr;
}

void *lock_backward(void *arguments)
{
	int i{}, iterate{}, backoffs{};
	int status{};
	for (iterate = 0; iterate < ITERATIONS; ++iterate) {
		backoffs = 0;
		for (i = 2; i > -1; --i) {
			if (i == 2)
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
					for (; i < 3; i++) {
						status = pthread_mutex_unlock(&mutex_array[i]);
						if (status != 0)
							err_abort(status, "Error during BACKOFF!");
					}
				}
				else {
					if (status != 0)
						err_abort(status, "Error while locking mutex");
					DPRINTF ((" forward locker got mutex %d locked\n", i));
				}
			}
			if (yield_flag) {
				if (yield_flag > 0)
					sched_yield();
				else
					sleep(1);
			}

		}
		printf("lock_backward got all locks, %d backoffs \n", backoffs);
		pthread_mutex_unlock(&mutex_array[0]);
		pthread_mutex_unlock(&mutex_array[1]);
		pthread_mutex_unlock(&mutex_array[2]);
		sched_yield();
	}
	return nullptr;
}

int main(int number_of_arguments, char *arguments[])
{
	pthread_t forward_thread{}, backward_thread{};
	int status{};
	if (number_of_arguments > 1)
		backoff = atoi(arguments[1]);

	if (number_of_arguments > 2)
		yield_flag = atoi(arguments[2]);
	status = pthread_create(&forward_thread, nullptr, lock_forward, nullptr);
	if (status != 0)
		err_abort(status, "Error while crating forward thread!");
	status = pthread_create(&backward_thread, nullptr, lock_backward, nullptr);
	if (status != 0)
		err_abort(status, "Error while crating backward thread!");
	pthread_exit(nullptr);
}