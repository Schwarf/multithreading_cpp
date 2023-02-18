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
			sleep_time = 1;
		else
			{
				alarm_list = alarm->link;
				now = time(nullptr);
				if(alarm->time <= now)
					sleep_time = 0;
				else
					sleep_time = alarm->time - now;
#ifdef DEBUG
		printf("[waiting: %d(%d)\"%s\"]\n", alarm->time, sleep_time, alarm->message);
#endif
		}
		status = pthread_mutex_unlock(&alarm_mutex);
		if(status != 0)
			err_abort(status, "Error while unlocking mutex");
		if(sleep_time > 0)
			sleep(sleep_time);
		else
			sched_yield();

		if(alarm != nullptr) {
			printf("(%d) %s\n", alarm->seconds, alarm->message);
			free(alarm);
		}
	}

}

int main(int number_of_arguments, char * argument_list[])
{
	int status;
	char line[128];
	alarm_tag *alarm, **last, *next;
	pthread_t thread;
	status = pthread_create(&thread, nullptr, alarm_thread, nullptr);
	if(status != 0)
		err_abort(status, "Error while creating alarm thread");
	while(true) {
		printf("Alarm >");
		if (fgets(line, sizeof(line), stdin) == NULL)
			exit(0);
		if (strlen(line) <= 1)
			continue;
		alarm = (alarm_tag *)malloc(sizeof(alarm_tag));
		if (alarm == nullptr)
			errno_abort("Error in allocating alarm");
		if (sscanf(line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2) {
			fprintf(stderr, "Bad command \n");
			free(alarm);
		}
		else
		{
			status = pthread_mutex_lock(&alarm_mutex);
			if (status != 0)
				err_abort(status, "Error while lock mutex to set alarm time");
			alarm->time = time (nullptr) + alarm->seconds;
			last = &alarm_list;
			next = *last;
			while(next != nullptr)
			{
				if(next->time >= alarm->time)
				{
					alarm->link = next;
					*last = alarm;
					break;
				}
				last = &next->link;
				next = next->link;
			}
			if(next == nullptr)
			{
				*last = alarm;
				alarm->link = nullptr;
			}
#ifdef DEBUG
			printf("list: ");
			for(next = alarm_list; next != nullptr; next = next->link)
				printf("%d(%d)[\"%s]\"]) ", next->time, next->time - time(nullptr), next->message);
			printf("]\n");
#endif
			status = pthread_mutex_unlock (&alarm_mutex);
			if (status != 0)
				err_abort (status, "Error while unlock mutex to set alarm time");
		}
	}
}
