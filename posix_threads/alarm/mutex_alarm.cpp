//
// Created by andreas on 18.02.23.
//

#include <pthread.h>
#include <ctime>
#include "errors.h"

struct alarm_tag{
	 alarm_tag * link;
	 int seconds;
	 time_t time;
	 char message[64];
};

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_tag *alarm_list = nullptr;


void *alarm_thread(void *arguments)
{
	alarm_tag * alarm;
	int sleep_time;
	time_t now;
	int status;
	while(true)
	{
		status = pthread_mutex_lock(&alarm_mutex);
		if(status != 0)
			err_abort(status, "Lock on mutex failed");
		alarm = alarm_list;

		if(alarm == nullptr)
	}

}
