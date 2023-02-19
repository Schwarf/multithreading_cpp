//
// Created by andreas on 18.02.23.
//
#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long counter{};

time_t end_time;

void *counter_thread(void *arg)
{
	int status{};
	int spin;
	while (time(nullptr) < end_time) {
		status = pthread_mutex_lock(&mutex);
		if (status != 0)
			err_abort(status, "Error while locking mutex");
		for (spin = 0; spin < SPIN; ++spin)
			counter++;
		status = pthread_mutex_unlock(&mutex);
		if (status != 0)
			err_abort(status, "Error while unlocking mutex");
		sleep(1);
	}
	printf("Counter is %#1x\n", counter);
	return nullptr;
}

void *monitor_thread(void *arg)
{
	int status{};
	int misses{};
	while (time(nullptr) < end_time) {
		sleep(3);
		status = pthread_mutex_trylock(&mutex);
		if (status != EBUSY) {
			if (status != 0)
				err_abort(status, "Trylock mutex in monitor");
			printf ("Counter is %ld\n",  counter/SPIN);
			status = pthread_mutex_unlock (&mutex);
			if (status != 0)
				err_abort(status, "Error while unlocking mutex in monitor");
		}
		else
			misses++;
	}
	printf("Monitor thread missed update %d times.\n", misses);
	return nullptr;
}

int main(int number_of_arguments, char *argument_list[])
{
	int status{};

	pthread_t counter_thread_id{};
	pthread_t monitor_thread_id{};

	end_time = time(nullptr) + 60;
	status = pthread_create (
		&counter_thread_id, nullptr, counter_thread, nullptr);
	if (status != 0)
		err_abort (status, "Error while creating counter thread");
	status = pthread_create (
		&monitor_thread_id, nullptr, monitor_thread, nullptr);
	if (status != 0)
		err_abort (status, "Error while creating monitor thread");
	status = pthread_join (counter_thread_id, nullptr);
	if (status != 0)
		err_abort (status, "Error while join counter thread");
	status = pthread_join (monitor_thread_id, nullptr);
	if (status != 0)
		err_abort (status, "Error while join monitor thread");
	return 0;
}

