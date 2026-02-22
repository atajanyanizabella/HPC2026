#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SIZE 50000000

int array[SIZE];
int partial_max[4];

void* max_element(void* arg) {
	int thread_id = *((int*)arg);

	int max_element = 0;
	for (int i = thread_id * (SIZE / 4); i < (thread_id + 1) * (SIZE / 4); ++i) {
		if (array[i] > max_element) {
			max_element = array[i];
		}
	}
	partial_max[thread_id] = max_element;
	return NULL;
} 

int main() {
	srand(time(NULL));	

	for (int i = 0; i < SIZE; ++i) {
		array[i] = rand();
	}

	clock_t seq_start, seq_end;
	// sequential approach
	int max = 0;
	seq_start = clock();
	for (int i = 0; i < SIZE; ++i) {
		if (array[i] > max) {
			max = array[i];
		}
	}
	seq_end = clock();
	double seq_time = (double)(seq_end - seq_start);
	printf("Sequential approach\n");
        printf("Max element = %d, Time Taken = %f\n", max, seq_time);

	// partial approach
	pthread_t threads[4];
	int thread_id[4] = {0, 1, 2, 3};
	clock_t start, end;
	
	start = clock();
	for (int i = 0; i < 4; ++i) {
		if (pthread_create(&threads[i], NULL, max_element, (void*)&thread_id[i]) != 0) {
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
	
	max = 0;
	for (int i = 0; i < 4; ++i) {
		if (partial_max[i] > max) {
			max = partial_max[i];
		}
	}
	end = clock();

	double partial_time = (double)(end - start);
	printf("Partial approach\n");
        printf("Max element = %d, Time Taken = %f\n", max, partial_time);
	return 0;
}
