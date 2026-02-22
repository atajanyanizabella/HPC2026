#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

#define ITERATIONS 10000000

void* func(void* arg) {
	int thread_id = *((int*)arg);
	for (int i = 0; i < ITERATIONS; ++i) {
		if (i % 2000000 == 0) {
			printf("Thread %d running on CPU %d\n", thread_id, sched_getcpu());
		}
	}
	return NULL;
}

int main() {
	int N = 0;
	printf("Enter thread count ");
	scanf("%d", &N);

	pthread_t threads[N];
	int thread_id[N];
	for (int i = 0; i < N; ++i) {
		thread_id[i] = i;
	}

	for (int i = 0; i < N; ++i) {
		if(pthread_create(&threads[i], NULL, func, (void*)&thread_id[i]) != 0) {
			printf("Thread%d", i + 1);
			perror(" creation failed\n");
			return 1;
		}
	}

	for (int i = 0; i < N; ++i) {
		if(pthread_join(threads[i], NULL) != 0) {
			printf("Thread%d", i + 1);
			perror(" creation failed\n");
			return 1;
		}
	}

	return 0;
}
