#include <stdio.h>

int main () {
	int var = 10;
	int* var_ptr = &var;

	printf("address of integer via variable: %p\n", &var);
	printf("address of integer via pointer: %p\n", var_ptr);

	*var_ptr = 15;

	printf("new value is %d\n", var);
	return 0;
}
