//#include <stdio.h>
//#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

mypthread_t tid[2];
int counter;
mypthread_mutex_t lock;

void* f(void* arg) {
  printf("Attempting thread execution...\n");
  mypthread_mutex_lock(&lock);

  /* CRITICAL SECTION */
  unsigned long i = 0;
  counter++;
  printf("Thread %d is in the critical section...\n", counter + 1);
  for(i = 0; i < (0xFFFFFFFF); i++); // dummy task
  printf("Thread %d is done!\n", counter);
  char* return_statement = "SUCCESS";
  /* CRITICAL SECTION */
  mypthread_mutex_unlock(&lock);
  mypthread_exit(return_statement);
}

int main(int argc, char **argv) {
  int i = 0;
  int err;

  if (mypthread_mutex_init(&lock, NULL) != 0) {
    printf("mypthread_mutex_init failed :(\n");
    return 1;
  }

  while (i < 2) {
    if (mypthread_create(&(tid[i]), NULL, &f, NULL) != 0) {
      printf("mypthread_create failed: [%s]\n", strerror(err));
    } else {
      printf("Main: Added thread %d to CPU.\n", i + 1);
    }
    i++;
  }

  void* returnValue;
  mypthread_join(tid[0], &returnValue);
  printf("Main: Thread 0 exited with the value: %s\n", returnValue);
  mypthread_join(tid[1], &returnValue);
  printf("Main: Thread 1 exited with the value: %s\n", returnValue);

  mypthread_mutex_destroy(&lock);

	return 0;
}
