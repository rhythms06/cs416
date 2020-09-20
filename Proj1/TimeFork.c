#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void timeFork(int numCalls); // Function Prototype

int main(int argc, char *argv[]){
	timeFork(5000);
	return 0;
}

void timeFork(int numCalls) {
	struct timeval start, end;
	int i;
	gettimeofday(&start, NULL);

	for(i = 0; i < numCalls; i++) {
		if (fork() == 0) { // If child, terminate
			exit(0);
		} else { // If parent, wait for child to terminate
			wait(NULL);
		}
	}
	gettimeofday(&end, NULL);

	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
	double avg = micros / ((double) numCalls);

	printf("Forks performed: %d\n", numCalls);
	printf("Total Elapsed Time: %ld microseconds\n", micros);
	printf("Average Time Per Fork: %f microseconds\n", avg);
}
