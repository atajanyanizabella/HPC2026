#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

#define N 100000000
#define HIST 256


int main () {
    int* A = (int*)malloc(N * sizeof(int));
    if (A == NULL) {
        perror("Failed to allocated memory\n");
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        A[i] = rand() % HIST;
    }

    int histogram[HIST];
    int GOLDEN[HIST];

    for (int i = 0; i < HIST; ++i) {
        histogram[i] = 0;
        GOLDEN[i] = 0;
    }

    for (int i = 0; i < N; ++i) {
        GOLDEN[A[i]]++;
    }

    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        histogram[A[i]]++;
    }

    for (int i = 0; i < HIST; ++i) {
        if (histogram[i] != GOLDEN[i]) {
            printf("Difference between NAIVE and GOLDEN\n");
            break;
        }
    }

    for (int i = 0; i < HIST; ++i) {histogram[i] = 0;}

    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        #pragma omp critical
        {
            histogram[A[i]]++;
        }
    }

    for (int i = 0; i < HIST; ++i) {
        if (histogram[i] != GOLDEN[i]) {
            printf("Difference between CRITICAL and GOLDEN\n");
            break;
        }
    }

    for (int i = 0; i < HIST; ++i) {histogram[i] = 0;}

    #pragma omp parallel for reduction(+:histogram[:HIST])
    for (int i = 0; i < N; ++i) {
        histogram[A[i]]++;
    }

    for (int i = 0; i < HIST; ++i) {
        if (histogram[i] != GOLDEN[i]) {
            printf("Difference between REDUCTION and GOLDEN\n");
            break;
        }
    }

    free(A);
    return 0;
}
