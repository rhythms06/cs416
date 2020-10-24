// File:	mypthread.c

// List all group member's name: Sakib Rasul and Sarah Squillace
// username of iLab: ses333
// iLab Server: prolog.cs.rutgers.edu

#include "mypthread.h"

// VARIABLES
bool firstThreadFlag = true;
tcb_queue* runqueue;
mypthread_t numThreads = -1;
tcb* currentThread;
ucontext_t* scheduler_context;
ucontext_t* current_thread_context;

// Scheduling timer
struct sigaction sa;
struct itimerval timer;

/* create a new thread (you can ignore attr) */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
  void *(*function) (void*), void * arg) {
  tcb* controlBlock = (tcb*) malloc(sizeof(tcb)); // a new thread control block
  if (firstThreadFlag) {
    initialize();
  }
//  else {
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

    numThreads++;
    controlBlock->id = numThreads; // save thread ID
    *thread = controlBlock->id;

    add_to_front(runqueue, controlBlock);
    // ^ Think controlBlock needs to be dynamically allocated...
  printf("queue after create\n");
  print_queue(runqueue);

//  }

  return 0;
};

void initialize() {
  firstThreadFlag = false;
  runqueue = (tcb_queue*) malloc(sizeof(tcb_queue));
  initialize_queue(runqueue);
  initialize_scheduler();
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
	printf("Yielding to the scheduler...\n");
  swapcontext(currentThread->context, scheduler_context);
  /* NOTE:
      I commented out the code above swapcontext because I was unsure if needed. That code was meant to
      "save context of this thread to its control block" but after looking at the man pages for swap context, it seems that saves the
      current context into the first argument.
  */
  return 0;
};

/* terminate a thread */
// Also deallocate any dynamic memory created when you started this thread
void mypthread_exit(void *value_ptr) {
  printf("Exit: thread %u with value %s\n", currentThread->id, (char *) value_ptr);

  if (value_ptr != NULL) {
    currentThread->returnValue = value_ptr;
  }

  currentThread->state = DONE;

  printf("Exit: Thread %u's state is currently %d\n", currentThread->id, currentThread->state);

  // might have to call schedule? what should currentThread be after this point? Since it's now pointing
  // to a block of memory that is not in use
  switch_to_scheduler(0);
}

/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {
  tcb* waited_on_tcb = find_tcb_by_id(thread);

  if (waited_on_tcb != NULL) {
    currentThread->waiting_on = waited_on_tcb;
    printf("Join: Thread %u is waiting on thread %u...\n", currentThread->id, waited_on_tcb->id);
    printf("Join: Thread %u's state is currently %d\n", waited_on_tcb->id, waited_on_tcb->state);

    if(waited_on_tcb->state != DONE) {
      // wait for the thread to terminate
      currentThread->state = WAITING;
      switch_to_scheduler(0);
    }
    // de-allocate any dynamic memory created by the joining thread
    //  free(waited_on_tcb->context);
    // make sure to return the return value of the exiting thread in value_ptr if not null
    *value_ptr = (void *) waited_on_tcb->returnValue;
    printf("Join: Saved return value %s\n", *(char**)value_ptr);

  } else { return -1; }

  return 0;
};

/* initialize the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
	bool* lock = (bool*) malloc(sizeof(bool));
	*lock = false;
  mutex->lock = lock;
	mutex->ownerControlBlock = currentThread;
	return 0;
};

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //
        // context switch to the scheduler thread
        if (__atomic_test_and_set (mutex->lock, 0) != true) {
          mutex->ownerControlBlock->state = BLOCKED;
          switch_to_scheduler(0);
        }
        // You're free to enter the critical section otherwise!
        return 0;
};

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex) {
	// Release mutex and make it available again.
	// Put threads in block list to run queue
	// so that they could compete for mutex later.
  mutex->lock = false;

  tcb_node* ptr = runqueue->front;
  while(ptr != NULL) {
    if(ptr->data->state == BLOCKED) {
      ptr->data->state = READY;
    }
    ptr = ptr->next;
  }

	return 0;
};


/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in mypthread_mutex_init
	mutex->lock = false;
  free(mutex->lock);
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
  controlBlock->state = RUNNING;
  controlBlock->counter = 0;
  controlBlock->id = 0;
  numThreads = controlBlock->id;

  currentThread = controlBlock;
  add_to_front(runqueue, controlBlock);
}


/* scheduler */

void switch_to_scheduler(int signum){
  printf("Switching to scheduler...\n");
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

#ifndef MLFQ
	  // Choose STCF
    sched_stcf();
#else
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
  while(1) {
    currentThread->counter++;

    move_min_to_back();

    if (runqueue->back->data->counter < currentThread->counter) {
      // enqueue old currentThread
      currentThread -> state = READY;
      add_to_front(runqueue, currentThread);
      // dequeue from runqueue and make it new currentThread
      currentThread = pop_from_back(runqueue);
      currentThread -> state = RUNNING;
      printf("Scheduler: Thread %u's state is currently %d\n", currentThread->id, currentThread->state);
    }

  // swap back to main context
  printf("Switching out of the scheduler...\n");
  swapcontext(scheduler_context, currentThread->context);
//  }
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
		printf("ID: %u\n", ptr->data->id);
    printf("STATE: %d\n", ptr->data->state);
    printf("COUNTER: %ld\n", ptr->data->counter);
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
  printf("Reorganizing queue...\n");

  tcb_node* ptr = runqueue->front;
  int min = INT_MAX;
  // First find id of min node
  while(ptr != NULL) {
    if (ptr->data->counter < min && ptr->data->state == READY) {
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
        runqueue->front->prev = NULL;
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
  ptr->next = NULL;
  runqueue->back = ptr;
  // cool we are done, now we can just do the pop from the back thing
}

void remove_from_queue(mypthread_t threadId) {
  tcb_node* ptr = runqueue->front;

  while(ptr != NULL) {
    if (ptr->data->id == threadId) {
      if (runqueue->front == ptr) {
        ptr->next->prev = NULL;
        runqueue->front = ptr->next;
      } else if (runqueue->back == ptr) {
        ptr->prev->next = NULL;
        runqueue->back = ptr->prev;
      } else {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
      }
    }
    ptr = ptr->next;
  }

  free(ptr);
}

void initialize_scheduler() {
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
}