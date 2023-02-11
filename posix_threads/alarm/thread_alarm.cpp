//
// Created by andreas on 11.02.23.
//
#include <pthread.h>
#include "errors.h"

struct AlarmTag{
	int seconds;
	char message[64];
};

void *alarm_thread(void * arg)
{
	auto * alarm = (AlarmTag*) arg;
	int status;

	status = pthread_detach(pthread_self());
	if(status != 0)
		err_abort(status, "Error while detaching thread");
	sleep(alarm->seconds);
	printf("(%d) %s \n", alarm->seconds, alarm->message);
	free (alarm);
	return nullptr;
}

