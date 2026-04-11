#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <float.h>

#define N 50000000

int main() {
    double *A = (double*)malloc(N * sizeof(double));
    if (A == NULL) {
        perror("Failed to allocate a memory\n");
        return 1;
    }

    for (long i = 0; i < N; i++) {
        A[i] = (double)rand() / RAND_MAX;
    }

    double max_val = -DBL_MAX;

    #pragma omp parallel for reduction(max:max_val)
    for (long i = 0; i < N; i++) {
        if (A[i] > max_val) {
            max_val = A[i];
        }
    }

    double T = 0.8 * max_val;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:sum)
    for (long i = 0; i < N; i++) {
        if (A[i] > T) {
            sum += A[i];
        }
    }

    printf("Max value: %f\n", max_val);
    printf("Threshold T: %f\n", T);
    printf("Filtered sum: %f\n", sum);

    free(A);
    return 0;
}
