// File:	mypthread.c

// List all group member's name: Sakib Rasul and Sarah Squillace
// username of iLab: // TODO
// iLab Server: // TODO

#include "mypthread.h"

// VARIABLES
bool firstThreadFlag = true;
tcb_queue* runqueue;
mypthread_t numThreads = -1;
mypthread_t currentThreadID;
tcb* currentThread;
ucontext_t* scheduler_context;
ucontext_t* current_thread_context;

// Scheduling timer
struct sigaction sa;
struct itimerval timer;

// Thread timer
//struct timeval threadStartTime;
//struct timeval threadEndTime;

/* create a new thread (you can ignore attr) */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
  void *(*function) (void*), void * arg) {
  tcb* controlBlock = (tcb*) malloc(sizeof(tcb)); // a new thread control block
  if (firstThreadFlag) {
    initialize();
  }

  ucontext_t *cp = (ucontext_t*) malloc(sizeof(ucontext_t)); // a new context pointer

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

  controlBlock->context = cp;
  controlBlock->wait_counter = 0;
  controlBlock->state = READY;

  // Modify the context
  controlBlock->context -> uc_link = scheduler_context; // assign the successor context
  controlBlock->context -> uc_stack.ss_sp = stack; // assign the context's stack
  controlBlock->context -> uc_stack.ss_size = STACK_SIZE; // the size of the new stack
  controlBlock->context -> uc_stack.ss_flags = 0;

  // Try applying our modifications
  errno = 0;
  makecontext(controlBlock->context, (void *)function, 1, arg);
  if (errno != 0) {
    perror("makecontext() reported an error");
    exit(1);
  }


  controlBlock->counter = 0;

  // TODO: Assign a new thread ID to controlBlock.
  numThreads++;
  controlBlock->id = numThreads; // save thread ID
  *thread = controlBlock->id;
  currentThreadID = controlBlock->id;

  // TODO: Enqueue thread onto a scheduler runqueue.
  add_to_front(runqueue, controlBlock);
  // ^ Think controlBlock needs to be dynamically allocated...
  


  return 0;
};

void initialize() {
  firstThreadFlag = false;
  runqueue = (tcb_queue*) malloc(sizeof(tcb_queue));
  initialize_queue(runqueue);

  /** ANNOYING OVERHEAD FOR CREATING A CONTEXT **/
  // TODO: Create Scheduler context
  scheduler_context = (ucontext_t*) malloc(sizeof(ucontext_t)); // a context pointer

  // Try to initialize context
  if (getcontext(scheduler_context) < 0) {
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
  scheduler_context->uc_link = NULL; // assign the successor context
  scheduler_context->uc_stack.ss_sp = stack; // assign the context's stack
  scheduler_context->uc_stack.ss_size = STACK_SIZE; // the size of the new stack
  scheduler_context->uc_stack.ss_flags = 0;

  // Try applying our modifications
  errno = 0;
  makecontext(scheduler_context, &schedule, 1, NULL);
  /** ANNOYING OVERHEAD FOR CREATING A CONTEXT **/
  init_main_thread(); // add the main thread to the scheduler

  initialize_timer();
}

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
  currentThread->state = READY;
	// save context of this thread to its thread control block
  // ucontext_t context; // a new thread context
  // ucontext_t *cp = &context; // a context pointer
  // if (getcontext(cp) < 0) {
  //   perror("getcontext() reported an error");
  //   exit(1);
  // }
  // currentTcb->context = context;
	// wwitch from thread context to scheduler context
  swapcontext(currentThread->context, scheduler_context);
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
	// tcb* currentTCB = find_tcb_by_id(currentThreadID);

	currentThread -> state = DONE;

	if (value_ptr != NULL) {
	    value_ptr = currentThread -> returnValue;
	}

	free(currentThread -> context);
  if (value_ptr == NULL) {
    // free and remove from queue
  }
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
  printf("Pausing thread %u...\n", currentThread->id);
  printf("Waiting on thread %u...\n", thread);

  // Set current status to wait
  // tcb* currentTcb = find_tcb_by_id(currentThread->id);
  currentThread->state = WAITING; // not sure if i should do this!!

  // Stop the thread's timer
//  gettimeofday(&threadEndTime, NULL);
//  long elapsedMicrosecondsSinceLastScheduled =
//          ((threadEndTime.tv_sec * 1000000 + threadEndTime.tv_usec) -
//           (threadStartTime.tv_sec * 1000000 + threadStartTime.tv_usec))
//          / QUANTUM;

  // Increment the thread's counter
//  currentTcb -> counter +=
//          elapsedMicrosecondsSinceLastScheduled;

  tcb* waited_on_tcb = find_tcb_by_id(thread);

	// wait for the thread to terminate
  while(waited_on_tcb->state != DONE);
  currentThread->state = RUNNING;

  // Start the new thread's timer.
//  gettimeofday(&threadStartTime, NULL);

	// de-allocate any dynamic memory created by the joining thread
  waited_on_tcb->wait_counter -= 1;
  // SEARCH THE THREAD IN THE QUEUE? DEALLOCATE THAT?
  // make sure to return the return value of the exiting thread in value_ptr if not null
  *value_ptr = waited_on_tcb->returnValue;

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

/* Initialize main thread */
void init_main_thread() {
  tcb* controlBlock = (tcb*) malloc(sizeof(tcb));

  ucontext_t *cp = (ucontext_t*) malloc(sizeof(ucontext_t)); // a new context pointer

  // Try to initialize context
  // if (getcontext(cp) < 0) {
  //   perror("getcontext() reported an error");
  //   exit(1);
  // }
//  current_thread_context = cp; // THIS LINE IS PROBABLY WRONG. MIGHT LEAD TO INF LOOP
  // Try allocating the context's stack
  // void *stack = malloc(STACK_SIZE);
  // if (stack == NULL) {
  //   perror("Could not allocate a new stack");
  //   exit(1);
  // }

  controlBlock->context = cp;
  controlBlock->wait_counter = 0;
  controlBlock->state = RUNNING;

  // Modify the context
  // controlBlock->context -> uc_link = NULL; // assign the successor context
  // controlBlock->context -> uc_stack.ss_sp = stack; // assign the context's stack
  // controlBlock->context -> uc_stack.ss_size = STACK_SIZE; // the size of the new stack
  // controlBlock->context -> uc_stack.ss_flags = 0;

  // Try applying our modifications
  // errno = 0;
  // makecontext(controlBlock.context, (void *)function, 1, arg);
  // if (errno != 0) {
  //   perror("makecontext() reported an error");
  //   exit(1);
  // }

  controlBlock->counter = 0;
  controlBlock->id = 0;
  numThreads = controlBlock->id;

  // TODO: Enqueue thread onto a scheduler runqueue.
  currentThread = controlBlock;
  add_to_front(runqueue, controlBlock);
}


/* scheduler */

void switch_to_scheduler(int signum){
  printf("Switching to scheduler");
  swapcontext(currentThread->context, scheduler_context);
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

	printf("Starting timer...\n");
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

//  gettimeofday(&threadEndTime, NULL);

//  long elapsedMicrosecondsSinceLastScheduled =
//          ((threadEndTime.tv_sec * 1000000 + threadEndTime.tv_usec) -
//           (threadStartTime.tv_sec * 1000000 + threadStartTime.tv_usec))
//           / QUANTUM;

//   currentThread -> counter +=
//           elapsedMicrosecondsSinceLastScheduled;

  // Increment counter
  currentThread->counter++;

  move_min_to_back();

  if (runqueue->back->data->counter < currentThread->counter) {
    // enqueue old currentThread
    add_to_front(runqueue, currentThread);
    // dequeue from runqueue and make it new currentThread
    currentThread = pop_from_back(runqueue);
    // If the minimum counter meets/exceeds the max quantum...
    // if (currentThread -> counter >= MAX_COUNTER) {
    //     // ...reset the thread's counter.
    //     currentThread -> counter = 0;
    // }
    // Start recording the thread's runtime.
//    gettimeofday(&threadStartTime, NULL);
  }

  // swap back to main context
  swapcontext(scheduler_context, currentThread->context);

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

	tcb_node* new_tcb_node =  malloc(sizeof(tcb_node));
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
  // First find id of min node
  while(ptr != NULL) {
    if (ptr->data->counter < min) {
      min = ptr->data->counter;
    }
		ptr = ptr->next;
	}

  // Then find id and move it to the back
  ptr = runqueue->front;
  while(ptr != NULL) {
    if (ptr->data->counter == min) {

      // Case 1: node at end of the list
      if (runqueue->back == ptr) {
        // we actually don't have to do anything in this case, we can just return
        return;
      }

      // Case 1: node is in the front of the list
      else if (runqueue->front == ptr) {
        runqueue->front = ptr->next;
        ptr->prev = runqueue->back;
        ptr->next->prev = NULL;
        break;
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
