#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

int numExceptions = 0;
int maxExceptions = 100000;

struct timeval start, end;

void handle_sigfpe(int signum) {
	numExceptions++;

	if (numExceptions == maxExceptions) {
		gettimeofday(&end, NULL); // stop timer

		long elapsedMicroseconds = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
		double averageTime = elapsedMicroseconds / ((double) numExceptions);

		printf("Exceptions Occurred: %d\n", numExceptions);
		printf("Total Elapsed Time: %ld microseconds\n", elapsedMicroseconds);
		printf("Average Time Per Exception: %f microseconds\n", averageTime);

		exit(0);
	}

	return;
}

int main(int argc, char *argv[]) {
	signal(SIGFPE, handle_sigfpe); // Register signal handler

	int x = 5;
	int y = 0;
	int z = 0;

	gettimeofday(&start, NULL); // start timer
	z = x / y; // invoke SIGFPE
	return 0;
}
