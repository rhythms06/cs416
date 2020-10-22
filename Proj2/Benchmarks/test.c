//#include <stdio.h>
//#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

 void* f1(void* arg) {
	 printf("Hi, I'm function 'f1'.");
	 return arg;
 }

int main(int argc, char **argv) {
	mypthread_t t1;
	mypthread_create(&t1, NULL, &f1, NULL);
	printf("Added Thread %u to CPU.", t1);
	return 0;
}
