#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SIZE 20000000

int array[SIZE];
int partial_max[4];

void* prime_numbers(void* arg) {
	int thread_id = *((int*)arg);

	int prime_count = 0;
	for (int i = thread_id * (SIZE / 4); i < (thread_id + 1) * (SIZE / 4); ++i) {
		if (array[i] <= 1) {
			continue;
		}
		for (int j = 2; j * j <= array[i]; ++j) {
			if (array[i] % j == 0) {
				continue;
			}
		}
		prime_count++;
	}
	partial_max[thread_id] = prime_count;
	return NULL;
} 

int main() {
	srand(time(NULL));	

	for (int i = 1; i <= SIZE; ++i) {
		array[i] = i;
	}

	clock_t seq_start, seq_end;
	// sequential approach
	int prime_count = 0;
	seq_start = clock();
	for (int i = 0; i < SIZE; ++i) {
		if (array[i] <= 1) {
			continue;
		}
		for (int j = 2; j * j <= array[i]; ++j) {
			if (array[i] % j == 0) {
				continue;
			}
		}
		prime_count++;
	}
	seq_end = clock();
	double seq_time = (double)(seq_end - seq_start);
	printf("Sequential approach\n");
        printf("Prime count = %d, Time Taken = %f\n", prime_count, seq_time);

	// partial approach
	pthread_t threads[4];
	int thread_id[4] = {0, 1, 2, 3};
	clock_t start, end;
	
	start = clock();
	for (int i = 0; i < 4; ++i) {
		if (pthread_create(&threads[i], NULL, prime_numbers, (void*)&thread_id[i]) != 0) {
			printf("Thread%d ", i + 1);
			perror("creation has been failed");
			return 1;
		}
	}
	
	for (int i = 0; i < 4; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			printf("Thread%d ", i + 1);
			perror("join has been failed");
			return 1;
		}
	}
	
	prime_count = 0;
	for (int i = 0; i < 4; ++i) {
		prime_count += partial_max[i];
	}
	end = clock();

	double partial_time = (double)(end - start);
	printf("Partial approach\n");
        printf("Prime count = %d, Time Taken = %f\n", prime_count, partial_time);
	return 0;
}
