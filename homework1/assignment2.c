#include "stdio.h"

int main () {
	int arr[5] = {1, 2, 3, 4, 5};
	int* arr_ptr = arr;

	for (int i = 0; i < 5; ++i) {
		printf("%d ", *(arr_ptr + i));
		*(arr_ptr + i) *= *(arr_ptr + i);
	}

	printf("\n");

	for (int i = 0; i < 5; ++i) {
		printf("%d | ", 	*(arr_ptr + i));
		printf("%d	", 	arr[i]);
	}

	printf("\n");

	return 0;
}
