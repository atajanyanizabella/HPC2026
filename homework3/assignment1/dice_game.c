#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_N 2
#define ROUNDS 5

pthread_barrier_t barrier;
int rolled_numbers[THREAD_N];
int winners[THREAD_N];

void* roller(void* args) {
	int id = *((int*)args);

	for (int r = 0; r < ROUNDS; ++r) {
		rolled_numbers[id] = rand() % 6 + 1;
		printf("Round %d: Player %d rolled %d\n", r + 1, id + 1, rolled_numbers[id]);

		printf("Round %d: Player %d is waiting...\n", r + 1, id + 1);
		int ret = pthread_barrier_wait(&barrier);
		if (ret == PTHREAD_BARRIER_SERIAL_THREAD) {
			printf("Round %d: Player %d is calculating max...\n", r + 1, id + 1);
			int max = 0;
			int winner_id = 0;
			for (int i = 0; i < THREAD_N; ++i) {
				if (rolled_numbers[i] > max) {
					max = rolled_numbers[i];
					winner_id = i;
				}
			}
			winners[winner_id]++;
			printf("Round %d winner is %d\n", r + 1, winner_id + 1);
		}
		//printf("Player %d: Waiting at the barrier for the second round...\n", id + 1);
		//pthread_barrier_wait(&barrier);
	}
	return NULL;
}

int main() {
	pthread_t players[THREAD_N];
	int ids[THREAD_N];
	for (int i = 0; i < THREAD_N; ++i) {
		rolled_numbers[i] = 0;
		winners[i] = 0;
		ids[i] = i;
	}

	pthread_barrier_init(&barrier, NULL, THREAD_N);
	int i = 0;
	for (i = 0; i < THREAD_N; ++i) {
		if (pthread_create(&players[i], NULL, &roller, (void*)(&ids[i])) != 0) {
			perror("Failed to create thread");
		}
	}
	
	for (i = 0; i < THREAD_N; ++i) {
		if (pthread_join(players[i], NULL) != 0) {
			perror("Failed to join thread");
		}
	}
	int winner_id = -1;
	int max_wins = 0;
	for (i = 0; i < THREAD_N; ++i) {
		if (winners[i] > max_wins) {
			max_wins = winners[i];
			winner_id = i;
		}
	}
	printf("MAIN THREAD: The winner is %d with %d wins!!!\n", winner_id + 1, max_wins);
	pthread_barrier_destroy(&barrier);
	return 0;
}
