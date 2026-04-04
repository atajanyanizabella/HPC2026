#include <stdio.h>
#include <cstdlib>
#include <omp.h>

struct order_t {
	int order_id;
	int distance_km;
	int priority;
};


int main() {
	int N = 10000;
	int threshold = 20;
	struct order_t orders[N];
	int thread_high_count[4];

	for (int i = 0; i < 4; ++i) {
		thread_high_count[i] = 0;
	}

	#pragma omp parallel num_threads(4)
	{
		int tid = omp_get_thread_num();
		#pragma omp single
		{
			for (int i = 0; i < N; ++i) {
				orders[i].order_id = i;
				orders[i].distance_km = rand() % 40;
			}
		}
		#pragma omp barrier

		#pragma omp for
		for (int i = 0; i < N; ++i) {
			if (orders[i].distance_km < threshold) {
				// 0 <=> "HIGH"
				orders[i].priority = 0;
			} else {
				// 1 <=> "NORMAL"
				orders[i].priority = 1;
			}
		}
		#pragma omp barrier

		#pragma omp single
		{
			printf("Priority assignment is FINISHED\n");
		}

		int counter = 0;
		#pragma omp for
		for (int i = 0; i < N; ++i) {
			if (orders[i].priority == 0)
				counter += 1;
		}
		
		thread_high_count[tid] = counter;
		
		#pragma omp barrier

		#pragma omp single
		{
			int sum_count = 0;
			for (int i = 0; i < 4; ++i) {
				int thread_count = thread_high_count[i];
				printf("Thread%d count = %d\n", i + 1, thread_count);
				sum_count += thread_count;
			}
			printf("Total count = %d\n", sum_count);
		}
	}
	return 0;
}
