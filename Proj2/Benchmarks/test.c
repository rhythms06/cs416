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

mypthread_t t1;
mypthread_t t2;

static void* f1(void* arg) {
  printf("Hi, I'm function 'f1'.");
  void* returnValue;
  mypthread_join(t2, &returnValue);
  printf("The thread returned the value: %s\n", (char*) returnValue);
  free(returnValue);
  return arg;
}

static void* f2(void* arg) {
  printf("Hi, I'm function 'f2'.");
  void* returnValue;
  mypthread_join(t1, &returnValue);
  printf("The thread returned the value: %s\n", (char*) returnValue);
  free(returnValue);
  return arg;
}

int main(int argc, char **argv) {
	mypthread_t t1;
	mypthread_create(&t1, NULL, &f1, "f1 has returned.");
  printf("Added thread %u to CPU.\n", t1);

  mypthread_t t2;
  mypthread_create(&t2, NULL, &f2, "f2 has returned.");
  printf("Added thread %u to CPU.\n", t2);

	return 0;
}
