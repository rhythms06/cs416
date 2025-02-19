#include "my_vm.h"

void* phys_mem;
bool first_call = true;
pde_t* page_dir;

struct tlb* cache_front;
struct tlb* cache_back;
int cache_size;
int hit_num;
int miss_num;

int offset_bits, page_dir_bits, page_table_bits;
int page_dir_size, page_table_size;
bool* phys_bitmap;
bool* virt_bitmap;

pthread_mutex_t lock;

/*
Function responsible for allocating and setting your physical memory
*/
void SetPhysicalMem() {
    // Initialize physical and virtual page bitmaps
    init_bitmaps();
    cache_front = NULL;
    cache_back = NULL;
    cache_size = 0;
    hit_num = 0;
    miss_num = 0;

    // Initialize Mutex
    if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
    } 

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
 * Adds a virtual-page-to-physical-page address translation to the TLB,
 * and evicts the oldest TLB entry if necessary.
 */
void
put_in_tlb(void *va, void *pa)
{
    // If the TLB is full, then evict its oldest entry.
    if (cache_size >= TLB_SIZE) {
        pop_from_back();
    }
    // Create a new TLB entry.
    struct tlb *new_entry = (struct tlb*) malloc(sizeof(struct tlb));
    new_entry->valid = 1;
    new_entry->virtual_page_number = (unsigned short)((unsigned int)va / PGSIZE);
    new_entry->physical_page_number = (unsigned short)((pa - phys_mem) / PGSIZE);
    // Add the new entry to the front of the TLB.
    add_to_front(new_entry);
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_in_tlb(void *va) {
    unsigned int page_num = (unsigned int) va / PGSIZE;
    unsigned int offset = get_offset(va);
    /* Part 2: TLB lookup code here */
    struct tlb* ptr = cache_front;
    while (ptr != NULL) {
        if (ptr->virtual_page_number == page_num) {
            hit_num++;
            return (void*) (((ptr->physical_page_number * PGSIZE) + phys_mem) + offset);
        }
        ptr = ptr->next;
    }
    miss_num++;
    return NULL;
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = (float) miss_num / (float) (miss_num + hit_num);
    miss_rate *= 100;
    fprintf(stderr, "TLB miss rate: %.2lf%%\n", miss_rate);
}


/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t * Translate(pde_t *pgdir, void *va) {
    // Get page directory, page table, and offset indices using the virtual address
    int offset = get_offset(va);
    if (USE_TLB) {
        void* pa_c = check_in_tlb(va);
        if (pa_c != NULL) {
            return pa_c ;
        }
    }

    int pgdir_index = get_outer_dex(va);
    int pgtbl_index = get_inner_dex(va);
    
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
    if (USE_TLB) {
        put_in_tlb(va, pa);
    }
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
    unsigned long long start_ptr, end_ptr; // when these pointers are at the start and end of a free block of contig pages
    // Then will be done with the algorithm
   
    start_ptr = 1;
    end_ptr = num_pages + start_ptr; 
    bool found_block = false; // Assume we have not found a free block yet
    for (start_ptr = 1; start_ptr < MAX_MEMSIZE / (unsigned long long)PGSIZE; ) {
        unsigned long long i;
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
    if (!found_block) {
        return NULL;
    }
    // virtual address = index of bit * pagesize
    int bitmap_indx = start_ptr;
    while (bitmap_indx < end_ptr) {
        virt_bitmap[bitmap_indx] = true; // Set the whole block to in use
        bitmap_indx++;
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
    pthread_mutex_lock(&lock);
    //HINT: If the physical memory is not yet initialized, then allocate and initialize.
    if (first_call) {
        SetPhysicalMem();
        first_call = false;
    }
    int num_pages = (int) ceil(((float)num_bytes) / ((float) PGSIZE));
    pte_t next_page = get_next_avail(num_pages); // Get next available free page 
    if (next_page == NULL) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }
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

    pthread_mutex_unlock(&lock);
    return next_page;
}

/*
 * If the memory block [va, va + size] can be freed, then free it and return 1.
 * Else, return -1.
*/
int myfree(void *va, int size) {
    pthread_mutex_lock(&lock);
    // Check if the memory block [va, va + size] can be freed
    for (int i = 0; i < size; i += PGSIZE) {
        if (page_dir[get_outer_dex(va + i)][get_inner_dex(va + i)] == NULL) {
            // Found a NULL page, which means the block can't be freed.
            // Return -1 to indicate failure.
            pthread_mutex_unlock(&lock);
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

    pthread_mutex_unlock(&lock);

    // Return 1 upon success.
    return 1;
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {

    pthread_mutex_lock(&lock);

    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/
    pte_t pa;
    pa = Translate(page_dir, va);

    unsigned int offset = get_offset(va);



    if (offset + size <= PGSIZE) { // The easy case where we are contained to one page
        memcpy((void*)(pa), val, size);
        pthread_mutex_unlock(&lock);
        return;
    }

    memcpy((void*)(pa), val, PGSIZE - offset); // copy for initial chunk
    unsigned int size_left = size - (PGSIZE - offset);
    va = va - offset + PGSIZE;
    val = val - offset + PGSIZE;
    while (size_left > 0) {
        unsigned int size_to_copy = size_left;
        if (size_left > PGSIZE) {
            size_to_copy = PGSIZE;
        }
        size_left -= size_to_copy;
        pa = Translate(page_dir, va);

        memcpy(pa, val, size_to_copy);
        va += PGSIZE;
        val += PGSIZE;
    }
    pthread_mutex_unlock(&lock);

}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {

    pthread_mutex_lock(&lock);
    /* HINT: put the values pointed to by "va" inside the physical memory at given
    "val" address. Assume you can access "val" directly by derefencing them.
    If you are implementing TLB,  always check first the presence of translation
    in TLB before proceeding forward */
    pte_t* pa = Translate(page_dir, va);

    unsigned int offset = get_offset(va);

    if (offset + size <= PGSIZE) { // The easy case where we are contained to one page
        memcpy(val, (void*)(pa), size);
        pthread_mutex_unlock(&lock);
        return;
    }

    memcpy(val, (void*)(pa), PGSIZE - offset); // copy for initial chunk
    unsigned int size_left = size - (PGSIZE - offset);
    va = va - offset + PGSIZE;
    val = val - offset + PGSIZE;
    while (size_left > 0) {
        unsigned int size_to_copy = size_left;
        if (size_left > PGSIZE) {
            size_to_copy = PGSIZE;
        }
        size_left -= size_to_copy;
        pa = Translate(page_dir, va);
        memcpy(val, pa, size_to_copy);
        va += PGSIZE;
        val += PGSIZE;
    }
    pthread_mutex_unlock(&lock);
}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void MatMult(void *mat1, void *mat2, int size, void *answer) {
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
    unsigned int mask = (unsigned int) (pow(2.0, page_table_bits) - 1);
    return ((unsigned int) va >> offset_bits) & mask;
}
unsigned int get_outer_dex(void * va) {
    unsigned int mask = (unsigned int) (pow(2.0, page_dir_bits) - 1);
    return ((unsigned int) va >> (offset_bits + page_table_bits)) & mask;
}

void init_bitmaps() {
    // Initialize bitmap of (MEMSIZE / PGSIZE) physical pages
    phys_bitmap = (bool*) calloc(MEMSIZE / PGSIZE, sizeof(bool));
    // Initialize bitmap of (MAX_MEMSIZE / PGSIZE) virtual pages
    virt_bitmap = (bool*) calloc(MAX_MEMSIZE / (unsigned long long) PGSIZE, sizeof(bool));
    // Set physical/virtual "failure indicators" to false
    phys_bitmap[0] = false;
    virt_bitmap[0] = false;
}

void initialize_tlb() {
	cache_front = NULL;
	cache_back = NULL;
	cache_size = 0;
}
void add_to_front(struct tlb* new_tlb) {

	new_tlb->next = cache_front;
	new_tlb->prev = NULL;

    if (cache_size < TLB_SIZE) {
        cache_size++;
    }
	

	if (cache_front == NULL) { // if queue is empty
		cache_front = new_tlb;
		cache_back = new_tlb;
		return;
	}

	cache_front->prev = new_tlb;
	cache_front = new_tlb;

	return;

}

struct tlb* pop_from_back() {
    if (cache_back == NULL) { // Queue is empty
		return NULL;
	}
	if (cache_back == cache_front) { // if only one in queue
		cache_front = NULL;
	} else { // if 2 or more in queue, we need to correct the value of next
		cache_back->prev->next = NULL;
	}
	struct tlb* popped = cache_back;
	cache_back = popped->prev;

	return popped;
}