#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define N 5

pthread_barrier_t barrier_1;
pthread_barrier_t barrier_2;
pthread_barrier_t barrier_3;

void* worker(void* arg) {
	int id = *((int*)arg);

	int sleep_time = 0;
	printf("Thread %d: Stage 1 started\n", id + 1);
	sleep_time = rand() % (2 * N) + 1;
	printf("Thread %d is sleeping for %d second...\n", id + 1, sleep_time);
	sleep(sleep_time);
	printf("Thread %d is ready and waiting\n", id + 1);
	pthread_barrier_wait(&barrier_1);

	printf("Thread %d: Stage 2 started\n", id + 1);
	sleep_time = rand() % (2 * N) + 1;
	printf("Thread %d is sleeping for %d second...\n", id + 1, sleep_time);
	sleep(sleep_time);
	printf("Thread %d is ready and waiting\n", id + 1);
	pthread_barrier_wait(&barrier_2);
		
	printf("Thread %d: Stage 3 started\n", id + 1);
	sleep_time = rand() % (2 * N) + 1;
	printf("Thread %d is sleeping for %d second...\n", id + 1, sleep_time);
	sleep(sleep_time);
	printf("Thread %d is ready and waiting\n", id + 1);
	pthread_barrier_wait(&barrier_3);

	return NULL;
}

int main() {
	pthread_t threads[N];
	int ids[N];

	pthread_barrier_init(&barrier_1, NULL, N);
	pthread_barrier_init(&barrier_2, NULL, N);
	pthread_barrier_init(&barrier_3, NULL, N);

	for (int i = 0; i < N; ++i) {
		ids[i] = i;
		if (pthread_create(&threads[i], NULL, worker, (void*)(&ids[i])) != 0) {
			perror("Failed to create thread\n");
		}
	}

	for (int i = 0; i < N; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Failed to join thread\n");
		}
	}

	pthread_barrier_destroy(&barrier_1);
	pthread_barrier_destroy(&barrier_2);
	pthread_barrier_destroy(&barrier_3);
	return 0;
}
