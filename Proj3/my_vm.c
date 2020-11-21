#include "my_vm.h"

void* phys_mem;
bool first_call = true;
pde_t* page_dir;
int offset_bits, page_dir_bits, page_table_bits;
int page_dir_size, page_table_size;
bool* phys_bitmap;
bool* virt_bitmap;

/*
Function responsible for allocating and setting your physical memory
*/
void SetPhysicalMem() {
    // Initialize physical and virtual page bitmaps
    init_bitmaps();
    // Allocate physical memory using malloc
    phys_mem = malloc(MEMSIZE);
    // Calculate number of page directory, page table, and offset bits
    offset_bits = (int) log2(PGSIZE);
    int VPN_bits = ADDRESS_BITS - offset_bits;
    if (VPN_bits % 2 != 0) {
        page_dir_bits = VPN_bits / 2 - 1;
    } else {
        page_dir_bits = VPN_bits / 2;
    }
    page_table_bits = VPN_bits / 2;
    // Calculate size of page directory and of each page table
    page_dir_size = (int) pow(2.0,  page_dir_bits);
    page_table_size = (int) pow(2.0, page_table_bits);
    // Allocate page directory using malloc (we'll also use malloc for each page table)
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
    // Get page directory, page table, and offset indices using the virtual address
    int pgdir_index = get_outer_dex(va);
    int pgtbl_index = get_inner_dex(va);
    int offset = get_offset(va);
    // Return NULL if page table doesn't exist
    if (pgdir[pgdir_index] == NULL) {
        return NULL;
    }
    // Return NULL if page table doesn't exist
    if (pgdir[pgdir_index][pgtbl_index] == NULL) {
        return NULL;
    }
    // Return the physical address
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
    // Get page directory and table indices
    int pgdir_index = get_outer_dex(va);
    int pgtbl_index = get_inner_dex(va);
    if (pgdir[pgdir_index] == NULL) { // If there's no page table here...
        pgdir[pgdir_index] = calloc(page_table_size, sizeof(pte_t)); // ...allocate one!
    }
    if (pgdir[pgdir_index][pgtbl_index] == NULL) { // If there's no table entry here...
        pgdir[pgdir_index][pgtbl_index] = pa; //...add one!
        return 1;
    }
    // Return -1 if a physical address already exists at the given table index.
    return -1;
}


/*
 * Function that gets the next available virtual page
*/
void *get_next_avail(int num_pages) {
    //Use virtual address bitmap to find the next free page
    unsigned int start_ptr, end_ptr; // when these pointers are at the start and end of a free block of contig pages
    // Then will be done with the algorithm
    end_ptr = num_pages; 
    start_ptr = 0;
    bool found_block = false; // Assume we have not found a free block yet
    for (start_ptr = 0; start_ptr < MAX_MEMSIZE / PGSIZE; ) {
        int i;
        for(i = start_ptr; i < end_ptr && i < MAX_MEMSIZE; i++) {

            if (virt_bitmap[i] == true) { // This block is not contiguous since there is a page in use
                start_ptr = i + 1; // Move start pointer to the page after the page in use
                end_ptr = start_ptr + num_pages;
                i = start_ptr - 1; // start i at the new start pointer
                continue; // Stop looking in this particular block of memory
            }
            if (i == end_ptr - 1 && virt_bitmap[i] == false) { // If we are at the end pointer and the end pointer is free
                found_block = true; // Then if we haven't been stopped yet, our block is free all the way!
                break;
            }
            // WARNING: this might infinitely loop I haven't checked it yet
        }

        if (found_block) {
            break;
        }
    }
    // virtual address = index of bit * pagesize
    while (start_ptr < end_ptr) {
        virt_bitmap[start_ptr] = true; // Set the whole block to in use
        start_ptr++;
    }
    return (void *) (start_ptr * PGSIZE);
}

/*
 * Returns pointer to next available physical page, or NULL if none exist.
 */
void* get_next_avail_phys() {
    // Initialize return page index
    unsigned int i = 1;
    // Look for next available physical page
    while (phys_bitmap[i] == true && i < MEMSIZE / PGSIZE) {
        i++;
    }
    // If i is less than the bitmap length, then...
    if (i < MEMSIZE / PGSIZE) {
        // ...an available page was found,
        // and its address is (i * page size + starting address of physical memory)!
        // Mark the page as occupied...
        phys_bitmap[i] = true;
        // ...and return its pointer.
        return (void *) (i * PGSIZE + phys_mem);
    }
    // Else, all pages are occupied.
    return NULL;
}

/* Function responsible for allocating pages
and used by the benchmark
*/
void *myalloc(unsigned int num_bytes) {

    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if (first_call) {
        SetPhysicalMem();
        first_call = false;
    }
    int num_pages = (int) ceil(((float)num_bytes) / ((float) PGSIZE));
    pte_t next_page = get_next_avail(num_pages); // Get next available free page 
    unsigned int i;
    pte_t current_page_addr;
    for (i = 0; i < num_pages; i++) {
        current_page_addr = next_page + (i * PGSIZE);
        void* pa = get_next_avail_phys();
        PageMap(page_dir, current_page_addr, pa);
    }
   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will
   have to mark which physical pages are used. */

    return next_page;
}

/*
 * If the memory block [va, va + size] can be freed, then free it and return 1.
 * Else, return -1.
*/
int myfree(void *va, int size) {
    // Check if the memory block [va, va + size] can be freed
    for (int i = 0; i < size; i += PGSIZE) {
        if (page_dir[get_outer_dex(va + i)][get_inner_dex(va + i)] == NULL) {
            // Found a NULL page, which means the block can't be freed.
            // Return -1 to indicate failure.
            return -1;
        }
    }
    // Free the memory block!
    for (int i = 0; i < size; i += PGSIZE) {
        // Get physical address of the current page.
        void * pa = page_dir[get_outer_dex(va + i)][get_inner_dex(va + i)];
        // Free the current page.
        page_dir[get_outer_dex(va + i)][get_inner_dex(va + i)] = NULL;
        // Mark the page as unoccupied in both bitmaps.
        virt_bitmap[((unsigned int)va + i) / PGSIZE] = false;
        phys_bitmap[((unsigned int)(pa - phys_mem))/ PGSIZE] = false;
    }
    // Return 1 upon success.
    return 1;
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {

    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/
    //int num_pages = (int) ceil(((float)num_bytes) / ((float) PGSIZE));
    unsigned int outer_indx = get_outer_dex(va);
    unsigned int inner_indx = get_inner_dex(va);
    unsigned int offset = get_offset(va);

    pte_t pa = page_dir[outer_indx][inner_indx];

    if (offset + size <= PGSIZE) { // The easy case where we are contained to one page
        memcpy((void*)(pa + offset), val, size);
        return;
    }

    memcpy((void*)(pa + offset), val, PGSIZE - offset); // copy for initial chunk
    // while (size > 0) {
    //     int chunk_size = 
    //     memcpy((void*)(pa + offset), val, size);
    // }

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
    // Allocate a 400-byte result matrix
    answer = myalloc(100*4);
    // Populate answer with the product of mat1 and mat2
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // Calculate (i, j)-entry of answer
            int entry = 0;
            for (int k = 0; k < size; k++) {
                int first, second;
                // Get the (i, k)-entry of mat1
                GetVal((void *) ((unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int))),
                       &first, sizeof(int));
                // Get the (k, j)-entry of mat2
                GetVal((void *) ((unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int))),
                       &second, sizeof(int));
                // Add the product of these entries to the (i, j)-entry of answer
                entry += first * second;
            }
            // Populate the (i, j)-entry of answer
            PutVal((void *)((unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int))),
                   &entry, sizeof(int));
        }
    }
}


unsigned int get_offset(void * va) {
    unsigned int mask = (unsigned int) (pow(2.0, offset_bits) - 1);
    return ((unsigned int)va) & mask;
}
unsigned int get_inner_dex(void * va) {
    unsigned int mask = (unsigned int) (pow(2.0, page_table_bits));
    return ((unsigned int) va >> offset_bits) & mask;
}
unsigned int get_outer_dex(void * va) {
    unsigned int mask = (unsigned int) (pow(2.0, page_dir_bits));
    return ((unsigned int) va >> offset_bits + page_table_bits) & mask;
}

void init_bitmaps() {
    // Initialize bitmap of (MEMSIZE / PGSIZE) physical pages
    phys_bitmap = (bool*) calloc(MEMSIZE / PGSIZE, sizeof(bool));
    // Initialize bitmap of (MAX_MEMSIZE / PGSIZE) virtual pages
    virt_bitmap = (bool*) calloc(MAX_MEMSIZE / PGSIZE, sizeof(bool));
    // Set physical/virtual "failure indicators" to false
    phys_bitmap[0] = false;
    virt_bitmap[0] = false;
}