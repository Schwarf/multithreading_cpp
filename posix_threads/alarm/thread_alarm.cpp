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

int main(int argc, char *argv[])
{
	int status;
	char line[128];
	AlarmTag * alarm;
	pthread_t thread;
	while(true)
	{
		printf("Alarm> ");
		if( fgets(line, sizeof(line), stdin) == nullptr)
			exit (0);
		if(strlen(line) <= 1)
			continue;
		alarm = (AlarmTag*)malloc(sizeof (AlarmTag) );
		if(!alarm)
			errno_abort("Error in allocating alarm-memory");
		if(sscanf (line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2)
		{
			fprintf(stderr, "Bad command \n");
			free(alarm);
		}
		else
		{
			status = pthread_create(&thread, nullptr, alarm_thread, alarm);
			if(status != 0)
				err_abort(status, "Error in creating alarm thread");
		}
	}

}