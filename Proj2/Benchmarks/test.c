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
 void* f1(void* p) {
	 printf("hello");
 }
int main(int argc, char **argv) {

	/* Implement HERE */
	printf("hello");
	mypthread_t t1 = NULL;
	mypthread_create(&t1, NULL, f1, NULL);
	return 0;
}
