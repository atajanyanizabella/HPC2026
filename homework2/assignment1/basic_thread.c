#include <stdio.h>
#include <pthread.h>

void* print_id(void* arg) {
	char* msg = (char*)arg;
	printf("%s\n", msg);
	return NULL;
}

int main() {
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	char* msg1 = "Thread1 is running";
	char* msg2 = "Thread2 is running";
	char* msg3 = "Thread3 is running";

	if (pthread_create(&thread1, NULL, print_id, (void*)msg1) != 0) {
		perror("Thread1 creation has been failed");
		return 1;
	}

	if (pthread_create(&thread2, NULL, print_id, (void*)msg2) != 0) {
		perror("Thread2 creation has been failed");
		return 1;
	}

	if (pthread_create(&thread3, NULL, print_id, (void*)msg3) != 0) {
		perror("Thread3 creation has been failed");
		return 1;
	}

	if (pthread_join(thread1, NULL) != 0) {
		perror("Thread1 join has been failed");
		return 1;
	}

	if (pthread_join(thread2, NULL) != 0) {
		perror("Thread2 join has been failed");
		return 1;
	}

	if (pthread_join(thread3, NULL) != 0) {
		perror("Thread3 join has been failed");
		return 1;
	}

	printf("All 3 threads have finished their execution\n");
	return 0;
}
