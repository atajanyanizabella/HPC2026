#include <stdio.h>
#include <cstdlib>

int main() {
	int* val = (int*)malloc(sizeof(int));
	if (val == NULL) {
		printf("memory allocation for value failed\n");
		return 1;
	}
	*val = 10;
	printf("val = %d\n", *val);

	int* arr = (int*)malloc(5 * sizeof(int));
	if (arr == NULL) {
		printf("memory allocation for array failed\n");
		return 1;
	}
	for (int i = 0; i < 5; ++i) {
		*(arr + i) = i;
		printf("%d ", arr[i]);
	}
	printf("\n");
	
	free(val);
	free(arr);
	
	return 0;
}
