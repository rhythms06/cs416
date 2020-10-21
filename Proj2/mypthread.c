// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "mypthread.h"

// VARIABLES
bool firstThreadFlag = true;
tcb_queue* runqueue;
mypthread_t currentThreadID;
ucontext_t scheduler_context;
ucontext_t main_thread_context;

// Timer global vars
struct sigaction sa;
struct itimerval timer;

/* create a new thread (you can ignore attr) */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
  void *(*function) (void*), void * arg) {
  tcb controlBlock; // a new thread control block
  if (firstThreadFlag) {
    initialize();
  }

  ucontext_t *cp = (ucontext_t*) malloc(sizeof(tcb)); // a new context pointer

  // Try to initialize context
  if (getcontext(cp) < 0) {
    perror("getcontext() reported an error");
    exit(1);
  }

  // Try allocating the context's stack
  void *stack = malloc(STACK_SIZE);
  if (stack == NULL) {
    perror("Could not allocate a new stack");
    exit(1);
  }

  controlBlock.context = cp;
  controlBlock.wait_counter = 0;
  controlBlock.state = READY;

  // Modify the context
  controlBlock.context -> uc_link = NULL; // assign the successor context
  controlBlock.context -> uc_stack.ss_sp = stack; // assign the context's stack
  controlBlock.context -> uc_stack.ss_size = STACK_SIZE; // the size of the new stack
  controlBlock.context -> uc_stack.ss_flags = 0;

  // Try applying our modifications
  errno = 0;
  makecontext(controlBlock.context, &function, 1, arg);
  if (errno != 0) {
    perror("makecontext() reported an error");
    exit(1);
  }


  controlBlock.counter = 0;

  // TODO: Enqueue thread onto a scheduler runqueue.
  add_to_front(runqueue, &controlBlock);
  // ^ Think controlBlock needs to be dynamically allocated...
  
  // TODO: Assign a new thread ID to controlBlock.
  *thread = controlBlock.id; // save thread ID

  return 0;
};

void initialize() {
  firstThreadFlag = false;
  runqueue = (tcb_queue*) malloc(sizeof(tcb_queue));
  initialize_queue(runqueue);

  /** ANNOYING OVERHEAD FOR CREATING A CONTEXT **/
  // TODO: Create Scheduler context
  ucontext_t context; // a new thread context
  ucontext_t *cp = &context; // a context pointer

  // Try to initialize context
  if (getcontext(cp) < 0) {
    perror("getcontext() reported an error");
    exit(1);
  }

  // Try allocating the context's stack
  void *stack = malloc(STACK_SIZE);
  if (stack == NULL) {
    perror("Could not allocated a new stack");
    exit(1);
  }

  // Modify the context
  context.uc_link = NULL; // assign the successor context
  context.uc_stack.ss_sp = stack; // assign the context's stack
  context.uc_stack.ss_size = STACK_SIZE; // the size of the new stack
  context.uc_stack.ss_flags = 0;

  // Try applying our modifications
  errno = 0;
  makecontext(&context, &schedule, 1, NULL);
  /** ANNOYING OVERHEAD FOR CREATING A CONTEXT **/
}

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

  // Find tcb with currentThreadID
  tcb* currentTcb = find_tcb_by_id(currentThreadID);
	// change thread state from Running to Ready
  currentTcb->state = READY;
	// save context of this thread to its thread control block
  // ucontext_t context; // a new thread context
  // ucontext_t *cp = &context; // a context pointer
  // if (getcontext(cp) < 0) {
  //   perror("getcontext() reported an error");
  //   exit(1);
  // }
  // currentTcb->context = context;
	// wwitch from thread context to scheduler context
  swapcontext((&(currentTcb->context)), &scheduler_context);
  /* NOTE:
      I commented out the code above swapcontext because I was unsure if needed. That code was meant to
      "save context of this thread to its control block" but after looking at the man pages for swap context, it seems that saves the
      current context into the first argument.
  */
  // TODO: create scheduler context (probably in initializer function)

	// YOUR CODE HERE
	return 0;
};

/* terminate a thread */
// Also deallocate any dynamic memory created when you started this thread
void mypthread_exit(void *value_ptr) {
	tcb* currentTCB = find_tcb_by_id(currentThreadID);

	currentTCB -> state = DONE;

	if (value_ptr != NULL) {
	    value_ptr = currentTCB -> returnValue;
	}

	free(currentTCB -> context);
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

  // Set current status to wait
  tcb* currentTcb = find_tcb_by_id(currentThreadID);
  currentTcb->state = WAITING; // not sure if i should do this!!
	// wait for a specific thread to terminate
  while (find_tcb_by_id(thread) != NULL);
  currentTCB->state = RUNNING; //

	// de-allocate any dynamic memory created by the joining thread

  // SEARCH THE THREAD IN THE QUEUE? DEALLOCATE THAT?
  // make sure to return the return value of the exiting thread in value_ptr if not null
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init

	return 0;
};

/* scheduler */

void switch_to_scheduler(int signum){
  printf("Switching to scheduler");
  swapcontext(&main_thread_context, &scheduler_context);
}

void initialize_timer(){
  // Set so that context switches to scheduler upon timer signal (SIGPROF)
  memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &switch_to_scheduler;
	sigaction (SIGPROF, &sa, NULL);

  timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 2;

	// Set up the current timer to go off in 1 second
	// Note: if both of the following values are zero
	//       the timer will not be active, and the timer
	//       will never go off even if you set the interval value
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

	// Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &timer, NULL);
}

static void schedule() {
	// Every time when timer interrup happens, your thread library
	// should be contexted switched from thread context to this
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (STCF or MLFQ)

	// if (sched == STCF)
	//		sched_stcf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose STCF
  sched_stcf();
#else
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)
  // find min counter
  // put the one with min counter to the back
  move_min_to_back();
  // swap back to main context
	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

/*** QUEUE FUNCTIONS ***/
void initialize_queue(tcb_queue* queue) {
  queue = (tcb_queue*) malloc(sizeof(tcb_queue));
	queue->front = NULL;
	queue->back = NULL;
	queue->size = 0;
}
void add_to_front(tcb_queue* queue, tcb* new_tcb) {
	printf("adding to front!\n");
	tcb_node* new_tcb_node = (tcb_node*) malloc(sizeof(tcb_node));
	new_tcb_node->data = new_tcb;
	new_tcb_node->next = queue->front;
	new_tcb_node->prev = NULL;

	if (queue->front == NULL) { // if queue is empty
		queue->front = new_tcb_node;
		queue->back = new_tcb_node;
		return;
	}

	queue->front->prev = new_tcb_node;
	queue->front = new_tcb_node;

	return;

}
tcb* pop_from_back(tcb_queue* queue) {
	if (queue->back == NULL) { // Queue is empty
		return NULL;
	}
	if (queue->back == queue->front) { // if only one in queue
		queue->front = NULL;
	} else { // if 2 or more in queue, we need to correct the value of next
		queue->back->prev->next = NULL;
	}
	tcb_node* popped = queue->back;
	queue->back = popped->prev;

	return popped->data;
}
void print_queue(tcb_queue* queue) {
	tcb_node* ptr = queue->front;
	while(ptr != NULL) {
		printf("%u\n", ptr->data->id);
		ptr = ptr->next;
	}
	return;
}
tcb* find_tcb_by_id(mypthread_t id) {
  tcb_node* ptr = runqueue->front;
  while(ptr != NULL) {
    if (ptr->data->id == id) {
      return ptr->data;
    }
		ptr = ptr->next;
	}
  return NULL;
}

void move_min_to_back() {
  tcb_node* ptr = runqueue->front;
  int min = INT_MAX;
  pthread_t min_id = NULL;
  // First find id of min node
  while(ptr != NULL) {
    if (ptr->data->counter < min) {
      min = ptr->data->counter;
      min_id = ptr->data->id;
    }
		ptr = ptr->next;
	}

  // Then find id and move it to the back
  ptr = runqueue->front;
  while(ptr != NULL) {
    if (ptr->data->id == min_id) {
      // Case 1: node is in the front of the list
      if (runqueue->front == ptr) {
        runqueue->front = ptr->next;
        ptr->next->prev = NULL;
        break;
      }
      // Case 2: node at end of the list
      else if (runqueue->back == ptr) {
        // we actually don't have to do anything in this case, we can just return
        return;
      }

      // Case 3: node is in the middle of the list
      else {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
        break;
      }

    }
		ptr = ptr->next;
	}
  // At this point, ptr should point to the node that we want to move to the back
  // so all we have to do is
  runqueue->back->next = ptr;
  runqueue->back = ptr;
  // cool we are done, now we can just do the pop from the back thing
}
