#include "my_vm.h"

void* phys_mem;
bool first_call = true;
pde_t* page_dir;
int offset_bits, page_dir_bits, page_table_bits;
int page_dir_size, page_table_size;
/*
Function responsible for allocating and setting your physical memory
*/
void SetPhysicalMem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    phys_mem = malloc(MEMSIZE);

    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
    // # virtual pages equivalent to MAX_MEMSIZE / PGSIZE
    // # physical pages equivalent to MEMSIZE / PGSIZE
    offset_bits = (int) log2(PGSIZE);
    // Initialize Virtual and Physical bitmap use the number of pages
    // Look up bitmap implementations online!
    int VPN_bits = ADDRESS_BITS - offset_bits;
    // Initialize page directory (allocate using malloc, you can also use malloc for the page tables themselves)
    // It's an array of size 2^(outer index)
    page_dir_bits;
    page_table_bits;

    if (VPN_bits % 2 != 0) {
        page_dir_bits = VPN_bits / 2 - 1;
    } else {
        page_dir_bits = VPN_bits / 2;
    }
    page_table_bits = VPN_bits / 2;

    page_dir_size = (int) pow(2.0,  page_dir_bits);
    page_table_size = (int) pow(2.0, page_table_bits);
    
    page_dir = (pde_t*) calloc(page_dir_size, sizeof(pde_t));

}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}


/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t * Translate(pde_t *pgdir, void *va) {
    //HINT: Get the Page directory index (1st level) Then get the
    //2nd-level-page table index using the virtual address.  Using the page
    //directory index and page table index get the physical address
    int pgdir_index = get_outer_dex(va);
    int pgtbl_index = get_inner_dex(va);
    int offset = get_offset(va);
    //If translation not successfull
    if (pgdir[pgdir_index] == NULL) { // If no page table has been allocated yet at this index
        return NULL;
    }
    if (pgdir[pgdir_index][pgtbl_index] == NULL) { // if there is no mapping
        return NULL;
    }
    void* pa = pgdir[pgdir_index][pgtbl_index];
    return pa + offset;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
PageMap(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to Translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    int pgdir_index = get_outer_dex(va);
    int pgtbl_index = get_inner_dex(va);

    if (pgdir[pgdir_index] == NULL) { // If no page table has been allocated yet at this index
        pgdir[pgdir_index] = calloc(page_table_size, sizeof(pte_t)); // allocate one!
    }
    if (pgdir[pgdir_index][pgtbl_index] == NULL) { // if there is no mapping
        pgdir[pgdir_index][pgtbl_index] = pa;
        return 1;
    }
    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {

    //Use virtual address bitmap to find the next free page

    // virtual address = index of bit * pagesize
}
// physical address = index of bit * pagesize + offset of the start of physical memory allocated

/* Function responsible for allocating pages
and used by the benchmark
*/
void *myalloc(unsigned int num_bytes) {

    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if (first_call) {
        SetPhysicalMem();
        first_call = false;
    }
   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will
   have to mark which physical pages are used. */

    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void myfree(void *va, int size) {

    //Free the page table entries starting from this virtual address (va)
    // Also mark the pages free in the bitmap
    //Only free if the memory from "va" to va+size is valid
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {

    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    "val" address. Assume you can access "val" directly by derefencing them.
    If you are implementing TLB,  always check first the presence of translation
    in TLB before proceeding forward */


}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void MatMult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
    matrix accessed. Similar to the code in test.c, you will use GetVal() to
    load each element and perform multiplication. Take a look at test.c! In addition to
    getting the values from two matrices, you will perform multiplication and
    store the result to the "answer array"*/


}


unsigned int get_offset(void * va) {
    unsigned int mask = (unsigned int) pow(2.0, offset_bits) - 1;
    return ((unsigned int)va) & mask;
}
unsigned int get_inner_dex(void * va) {
    unsigned int mask = (unsigned int) pow(2.0, page_table_bits);
    return ((unsigned int) va >> offset_bits) & mask;
}
unsigned int get_outer_dex(void * va) {
    unsigned int mask = (unsigned int) pow(2.0, page_dir_bits);
    return ((unsigned int) va >> offset_bits + page_table_bits) & mask;
}