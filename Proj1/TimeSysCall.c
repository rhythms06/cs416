#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void timeSysCall(int numCalls); // Function prototype
// Place any necessary global variables here

int main(int argc, char *argv[]){

	timeSysCall(100000);
	return 0;

}

void timeSysCall(int numCalls) {
	struct timeval start, end;
	int i;
	gettimeofday(&start, NULL);
	for(i = 0; i < numCalls; i++) {
		getpid();
	}
	gettimeofday(&end, NULL);
	
	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
	double avg = micros / ((double) numCalls);
	
	printf("Syscalls performed: %d\n", numCalls);
	printf("Total Elapsed Time: %ld microseconds\n", micros);
	printf("Average Time Per Syscall: %f microseconds\n", avg);
}