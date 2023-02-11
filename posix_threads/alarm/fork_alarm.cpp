//
// Created by andreas on 09.02.23.
//
#include <sys/types.h>
#include <wait.h>
#include "errors.h"

int main(int argc, char *argv[])
{
	int status;
	char line[128];
	int seconds;
	pid_t pid;
	char message[64];

	while (true) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == nullptr)
			exit(0);
		if (strlen(line) <= 1)
			continue;

		/*
		 * Parse input line into seconds (%d) and a message
		 * (%64[^\n]), consisting of up to 64 characters
		 * separated from the seconds by whitespace.
		 */
		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
			fprintf(stderr, "Bad command\n");
		}
		else {
			pid = fork();
			printf("PID = %d \n", pid);
			if (pid == (pid_t)-1)
				errno_abort ("Fork");
			if (pid == (pid_t)0) {
				/*
				 * If we're in the child, wait and then print a message
				 */
				sleep(seconds);
				printf("(%d) %s\n", seconds, message);
				exit(0);
			}
			else {
				/*
				 * In the parent, call waitpid() to collect any children that
				 * have already terminated.
				 */
				do {
					pid = waitpid((pid_t)-1, nullptr, WNOHANG);
					if (pid == (pid_t)-1)
						errno_abort ("Wait for child");
				}
				while (pid != (pid_t)0);
			}
		}
	}
}
