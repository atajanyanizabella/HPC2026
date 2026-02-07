#include <stdio.h>

int main() {
	const char* string_arr[] = {"engineering", "computer scienece", "data scienece"};
	for (int i = 0; i < 3; i++) {
		printf("%s\n", *(string_arr + i));
	}

	printf("\nModified\n\n");

	*(string_arr + 1) = "business";

	for (int i = 0; i < 3; i++) {
		printf("%s\n", *(string_arr + i));
	}
	return 0;	
}
