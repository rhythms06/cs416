
#include "../my_vm.h"
#include <time.h>

#define SIZE 5

// void main() {
//     void* p = myalloc(4096 + 4096 + 4);
//     char* c = (char*) malloc(4096 + 4096 + 4);

//     PutVal(p, c, 4096 + 4096 + 4);
// }

int main() {
    // Start recording runtime.
    clock_t start = clock();

    printf("Allocating three arrays of 400 bytes\n");
    void *a = myalloc(100*4);
    int old_a = (int)a;
    void *b = myalloc(100*4);
    void *c = myalloc(100*4);
    int x = 1;
    int y, z;
    int i =0, j=0;
    int address_a = 0, address_b = 0;
    int address_c = 0;

    printf("Addresses of the allocations: %x, %x, %x\n", (int)a, (int)b, (int)c);

    printf("Storing integers to generate a SIZExSIZE matrix\n");
    for (i = 0; i < SIZE; i++) {
        x = j + 1;
        for (j = 0; j < SIZE; j++) {
            x = j + 1;
            address_a = (unsigned int)a + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            PutVal((void *)address_a, &x, sizeof(int));
            PutVal((void *)address_b, &x, sizeof(int));
        }
    }

    printf("Fetching matrix elements stored in the arrays\n");

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            address_a = (unsigned int)a + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            address_b = (unsigned int)b + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_a, &y, sizeof(int));
            GetVal( (void *)address_b, &z, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    }

    printf("Performing matrix multiplication with itself!\n");
    MatMult(a, b, SIZE, c);


    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            address_c = (unsigned int)c + ((i * SIZE * sizeof(int))) + (j * sizeof(int));
            GetVal((void *)address_c, &y, sizeof(int));
            printf("%d ", y);
        }
        printf("\n");
    }
    printf("Freeing the allocations!\n");
    myfree(a, 100*4);
    myfree(b, 100*4);
    myfree(c, 100*4);

    printf("Checking if allocations were freed!\n");
    a = myalloc(100*4);
    if ((int)a == old_a)
        printf("free function works\n");
    else
        printf("free function does not work\n");

    // Stop recording runtime.
    clock_t end = clock();
    double runtime = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Total runtime: %.6f seconds\n", runtime);
    if (USE_TLB) {
        print_TLB_missrate();
    }
    return 0;
}


