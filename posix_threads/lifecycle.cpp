//
// Created by andreas on 12.02.23.
//
#include <pthread.h>
#include "errors.h"


void * method_called_by_thread(void * argument)
{
	return argument;
}

int main()
{
	pthread_t thread_id;
	int x =8;
	void * result;
	int status;

	status = pthread_create(& thread_id, nullptr, method_called_by_thread, nullptr);
	if(status)
		err_abort(status, "Error in create thread");

	status = pthread_join(thread_id, &result);
	if(status)
		err_abort(status, "Error in join thread");
	status = pthread_join(thread_id, &result);
	if(status)
		err_abort(status, "Error in join thread2 ");
	if(result == nullptr)
		return 0;
	else
		return 1;

};