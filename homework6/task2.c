#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <float.h>
#include <math.h>

#define N 50000000

int main() {
    double *A = (double*)malloc(N * sizeof(double));
    if (A == NULL) {
        perror("Failed to allocate memory\n");
        return 1;
    }

    for (long i = 0; i < N; i++) {
        A[i] = (double)rand() / RAND_MAX;
    }

    double min_diff = DBL_MAX;

    #pragma omp parallel for reduction(min:min_diff)
    for (long i = 1; i < N; i++) {
        double diff = fabs(A[i] - A[i - 1]);
        if (diff < min_diff) {
            min_diff = diff;
        }
    }

    printf("Minimum absolute difference: %f\n", min_diff);

    free(A);
    return 0;
}
