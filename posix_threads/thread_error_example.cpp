//
// Created by andreas on 12.02.23.
//
#include <pthread.h>
#include <cstdio>
#include <cstring>

int main()
{
	pthread_t thread;
	int status;
	status = pthread_join(thread, nullptr);
	if(status)
		fprintf(stderr, "error %d: %s\n", status, strerror (status));
	return  status;
}