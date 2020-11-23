#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096

// Maximum size of your memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024 //4GB
#define ADDRESS_BITS 32
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
// Domingo suggests
typedef void* pte_t;
// typedef unsigned long pte_t;

// Represents a page directory entry
// Domingo suggests
typedef pte_t* pde_t;
// typedef unsigned long pde_t;

#define TLB_SIZE 120

// The structure of a TLB entry
struct tlb {
    // set to 1 if holding a valid translation, 0 if not.
    int valid;
    // a virt_bitmap index
    int virtual_page_number;
    // a phys_bitmap index
    int physical_page_number;
    // Assume each bucket to be 4 bytes (???)
    struct tcb* next;
    struct tcb* prev;
};
struct tlb tlb_store;


void SetPhysicalMem();
pte_t* Translate(pde_t *pgdir, void *va);
int PageMap(pde_t *pgdir, void *va, void* pa);
bool check_in_tlb(void *va);
void put_in_tlb(void *va, void *pa);
void *myalloc(unsigned int num_bytes);
int myfree(void *va, int size);
void PutVal(void *va, void *val, int size);
void GetVal(void *va, void *val, int size);
void MatMult(void *mat1, void *mat2, int size, void *answer);
void print_TLB_missrate();
unsigned int get_offset(void * va);
unsigned int get_inner_dex(void * va);
unsigned int get_outer_dex(void * va);
void init_bitmaps();
/* QUEUE FUNCTIONS */
void add_to_front(struct tlb* new_entry);
struct tlb* pop_from_back();
void* find_page(void* va);

#endif
