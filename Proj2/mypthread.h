// File:	mypthread_t.h

// List all group member's name: Sakib Rasul and Sarah Squillace
// username of iLab: // TODO
// iLab Server: // TODO

#ifndef MYTHREAD_T_H
#define MYTHREAD_T_H

#define _GNU_SOURCE

// This line suppresses deprecation warnings on macOS
#define _XOPEN_SOURCE 600

/* To use Linux pthread Library in Benchmark, you have to comment the USE_MYTHREAD macro */
#define USE_MYTHREAD 1

/* include lib header files that you need here: */

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#define STACK_SIZE 200000
#define QUANTUM 5000 // The length of one quantum, in microseconds
#define MAX_COUNTER 1 // The number of quanta a thread is allowed to run for
typedef unsigned int mypthread_t;

enum state {RUNNING, READY, WAITING, START, DONE};

typedef struct threadControlBlock {
    // The thread's ID. Used to refer to the thread.
	mypthread_t id;
	// Thread state (running, ready, waiting, start, done)
	enum state state;
	// A pointer to the thread's context
	ucontext_t* context;
	// The number of time quanta the thread has run for so far (aka its priority)
	long counter;
	// The thread's return value.
	void* returnValue;
	// Counter to count the number of threads waiting on this thread
	struct threadControlBlock* waiting_on;
} tcb;

/* mutex struct definition */
typedef struct mypthread_mutex_t {
  bool* lock;
  tcb* ownerControlBlock;
} mypthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)
typedef struct tcb_node {
	tcb* data;
	struct tcb_node* next;
	struct tcb_node* prev;
} tcb_node;

typedef struct tcb_queue {
	tcb_node* front;
	tcb_node* back;
	int size;
} tcb_queue;

/* Function Declarations: */

/* create a new thread */
int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* initialize thread library */
void initialize();

/* give CPU pocession to other user level threads voluntarily */
int mypthread_yield();

/* terminate a thread */
void mypthread_exit(void *value_ptr);

/* wait for thread termination */
int mypthread_join(mypthread_t thread, void **value_ptr);

/* initial the mutex lock */
int mypthread_mutex_init(mypthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int mypthread_mutex_lock(mypthread_mutex_t *mutex);

/* release the mutex lock */
int mypthread_mutex_unlock(mypthread_mutex_t *mutex);

/* destroy the mutex */
int mypthread_mutex_destroy(mypthread_mutex_t *mutex);

/* MAIN THREAD */
void init_main_thread();

/* SCHEDULER FUNCTIONS */
static void schedule();
static void sched_stcf();
void switch_to_scheduler(int signum);
void initialize_timer();

/* QUEUE FUNCTIONS */
void initialize_queue(tcb_queue* queue);
void add_to_front(tcb_queue* queue, tcb* new_tcb);
tcb* pop_from_back(tcb_queue*);
void print_queue(tcb_queue* queue);
tcb* find_tcb_by_id(mypthread_t id);
void move_min_to_back();
void remove_from_queue(mypthread_t threadId);
void initialize_scheduler();

#ifdef USE_MYTHREAD
#define pthread_t mypthread_t
#define pthread_mutex_t mypthread_mutex_t
#define pthread_create mypthread_create
#define pthread_exit mypthread_exit
#define pthread_join mypthread_join
#define pthread_mutex_init mypthread_mutex_init
#define pthread_mutex_lock mypthread_mutex_lock
#define pthread_mutex_unlock mypthread_mutex_unlock
#define pthread_mutex_destroy mypthread_mutex_destroy
#endif

#endif
