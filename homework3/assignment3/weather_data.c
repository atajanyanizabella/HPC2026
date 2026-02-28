#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SENSORS 2
#define ROUNDS 5

pthread_barrier_t barrier;
int temperatures[SENSORS];
float averages[SENSORS];

void* roller(void* args) {
	int id = *((int*)args);

	for (int r = 0; r < ROUNDS; ++r) {
		temperatures[id] = 20.0 + (rand() % 1000) / 100.0;
		printf("Round %d: Sensor %d collected %d\n", r + 1, id + 1, temperatures[id]);

		printf("Round %d: Sensor %d is waiting...\n", r + 1, id + 1);
		int ret = pthread_barrier_wait(&barrier);
		if (ret == PTHREAD_BARRIER_SERIAL_THREAD) {
			printf("Round %d: Sensor %d is calculating average...\n", r + 1, id + 1);
			int sum = 0;
			for (int i = 0; i < SENSORS; ++i) {
				sum += temperatures[i];
			}
			averages[r] = sum / SENSORS;
			printf("Round %d average is %f\n", r + 1, averages[r]);
		}
		//printf("Sensor %d: Waiting at the barrier for the second round...\n", id + 1);
		//pthread_barrier_wait(&barrier);
	}
	return NULL;
}

int main() {
	pthread_t threads[SENSORS];
	int ids[SENSORS];
	for (int i = 0; i < SENSORS; ++i) {
		temperatures[i] = 0;
		averages[i] = 0;
		ids[i] = i;
	}

	pthread_barrier_init(&barrier, NULL, SENSORS);
	int i = 0;
	for (i = 0; i < SENSORS; ++i) {
		if (pthread_create(&threads[i], NULL, &roller, (void*)(&ids[i])) != 0) {
			perror("Failed to create thread");
		}
	}
	
	for (i = 0; i < SENSORS; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Failed to join thread");
		}
	}
	pthread_barrier_destroy(&barrier);
	return 0;
}
