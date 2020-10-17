#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"

/* A scratch program template on which to call and
 * test mypthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
int main(int argc, char **argv) {

	/* Implement HERE */
	tcb_queue* q = (tcb_queue*) malloc(sizeof(tcb_queue));
	initialize_queue(q);
	tcb* tcb1 = (tcb*) malloc(sizeof(tcb));
	tcb1->id = 1;
	tcb* tcb2 = (tcb*) malloc(sizeof(tcb));
	tcb2->id = 2;
	add_to_front(q, tcb1);
	add_to_front(q, tcb2);
	print_queue(q);
	return 0;
}
