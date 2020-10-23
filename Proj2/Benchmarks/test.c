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

static void* f1(void* arg) {
  printf("Hi, I'm function 'f1' and I was given the arg '%s'\n", *(char**)arg);
  char* exit_statement_f1 = "f1 has exited";
  mypthread_exit(&exit_statement_f1);
  char* return_statement_f1 = "f1 has returned";
  return return_statement_f1;
}

int main(int argc, char **argv) {
	mypthread_t t1;
	char* str = "f1's sole argument";
	mypthread_create(&t1, NULL, &f1, &str);
  printf("Added thread %u to CPU.\n", t1);

  void* returnValue;

  mypthread_join(t1, &returnValue);
  printf("The thread exited with the value: %s\n", *(char**) returnValue);

  free(returnValue);

	return 0;
}
