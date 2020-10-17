// File:	mypthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "mypthread.h"

// VARIABLES



/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr,
                      void *(*function)(void*), void * arg) {
       tcb controlBlock; // a new thread control block
       ucontext_t context; // a new thread context
       void *stack = malloc(STACK_SIZE); // allocate stack space
       context.uc_stack.ss_sp = stack; // the base of the new stack
       context.uc_stack.ss_size = STACK_SIZE; // the size of the new stack
       context.uc_link = NULL; // the successor stack
       context.uc_stack.ss_flags = 0;
	   makecontext(&context, *function, 1);
       controlBlock.context = context;
       // after everything is all set, push this thread int
       // YOUR CODE HERE

    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int mypthread_yield() {

	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
	return 0;
};

/* terminate a thread */
void mypthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr) {

	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread

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
#else
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

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
void initialize_queue(tcb_queue* queue) {
	queue->front = NULL;
	queue->back = NULL;
	queue->size = 0;
}
void add_to_front(tcb_queue* queue, tcb* new_tcb) {
	printf("adding to front!\n");
	tcb_node* new_tcb_node = (tcb_node*) malloc(sizeof(tcb_node));
	new_tcb_node->data = new_tcb;
	new_tcb_node->next = queue->front;

	if (queue->front == NULL) { // if queue is empty
		queue->front = new_tcb_node;
		queue->back = new_tcb_node;
		return;
	}

	
	queue->front = new_tcb_node;

	return;

}
void print_queue(tcb_queue* queue) {
	tcb_node* ptr = queue->front;
	while(ptr != NULL) {
		printf("%u\n", ptr->data->id);
		ptr = ptr->next;
	}
	return;
}