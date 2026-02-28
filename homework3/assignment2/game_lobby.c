#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define N 5

pthread_barrier_t barrier;

void* rand_sleep(void* arg) {
	int player_id = *((int*)arg);

	int ready_time = rand() % (2 * N) + 1;
	printf("Player %d is getting ready for %d second...\n", player_id, ready_time);
	sleep(ready_time);

	printf("Player %d is ready and waiting\n", player_id);

	pthread_barrier_wait(&barrier);
	printf("Player %d: Game Started\n", player_id);

	return NULL;
}

int main() {
	pthread_t threads[N];
	int ids[N];

	pthread_barrier_init(&barrier, NULL, N);

	for (int i = 0; i < N; ++i) {
		ids[i] = i;
		if (pthread_create(&threads[i], NULL, rand_sleep, (void*)(&ids[i])) != 0) {
			perror("Failed to create thread\n");
		}
	}

	for (int i = 0; i < N; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Failed to join thread\n");
		}
	}

	pthread_barrier_destroy(&barrier);
	return 0;
}
