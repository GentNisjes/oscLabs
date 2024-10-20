#include <stdio.h>
int a = 1;
int b = 2; // for testing we use pointers to integers
int *p = &a;
int *q = &b;

void swap_pointers(int** p1, int** p2) {
    int* temp = *p1 ;   //address of a in p1 in temp
    *p1 = *p2;          //
    *p2 = temp;
}

int main() {
    printf("Before swap:\n");
    printf("p points to: %p (value: %d)\n", (void*)p, *p); // Address of a, value: 1
    printf("q points to: %p (value: %d)\n", (void*)q, *q); // Address of b, value: 2

    // Swap the pointers
    swap_pointers(&p, &q);

    printf("After swap:\n");
    printf("p points to: %p (value: %d)\n", (void*)p, *p); // Address of b, value: 2
    printf("q points to: %p (value: %d)\n", (void*)q, *q); // Address of a, value: 1

    return 0;
}

