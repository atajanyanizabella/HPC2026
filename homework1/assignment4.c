#include <stdio.h>

int main() {
	int var = 10;
	int* var_ptr = &var;
	int** var_ptr_ptr = &var_ptr;

	printf("value of integer via pointer: %d\n", *var_ptr);
	printf("value of integer via double-pointer: %d\n", *(*var_ptr_ptr));
	return 0;
}
