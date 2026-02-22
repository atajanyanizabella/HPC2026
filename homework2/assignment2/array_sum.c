#include <stdio.h>
#include <pthread.h>

#define SIZE 50000000
#define TH_COUNT 4

int array[SIZE];
long partial_sums[TH_COUNT];

void* sum_array(void* arg) {
	int thread_id = *((int*)arg);

	long sum = 0;
	for (int i = thread_id * (SIZE / TH_COUNT); i < (thread_id + 1) * (SIZE / TH_COUNT); ++i) {
		sum += array[i];
	}

	partial_sums[thread_id] = sum;
	return NULL;
}

int main() {
	// initializing the array
	for (int i = 0; i < SIZE; ++i) {
		array[i] = 1;
	}

	long sequential_sum = 0;
	clock_t seq_start, seq_end;

	// sequential sum
	seq_start = clock();
	for (int i = 0; i < SIZE; ++i) {
		sequential_sum += array[i];
	}
	seq_end = clock();
	
	double seq_time = (double)(seq_end - seq_start);
	printf("Sequential approach\n");
	printf("Total sum = %ld, Time Taken = %f\n", sequential_sum, seq_time);
	
	// partial sum approach
	pthread_t threads[TH_COUNT];
	int thread_ids[TH_COUNT];
	for (int i = 0; i < TH_COUNT; ++i) {
		thread_ids[i] = i;
	}
	clock_t start, end;
	long total_sum = 0;

	start = clock();
	
	for (int thread_id = 0; thread_id < TH_COUNT; ++thread_id) {
		if(pthread_create(&threads[thread_id], NULL, sum_array, (void*)&thread_ids[thread_id]) != 0) {
			perror("Thread%d creation has been failed", thread_id + 1);
			return 1;
		}
	}

	for (int thread_id = 0; thread_id < TH_COUNT; ++thread_id) {
		if(pthread_join(threads[thread_id], NULL) != 0) {
			perror("Thread%d join has been failed", thread_id + 1);
			return 1;
		}
	}

	for (int i = 0; i < TH_COUNT; ++i) {
		total_sum += partial_sums[i];	
	}
	
	end = clock();

	double time_taken = (double)(end - start);
	printf("Partial approach\n");
	printf("Total sum = %ld, Time Taken = %f\n", total_sum, time_taken);
	return 0;
}
