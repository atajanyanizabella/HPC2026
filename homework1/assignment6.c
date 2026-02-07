#include <stdio.h>
#include <cstdlib>

int str_length(const char* str) {
	int len = 0;
	while (*(str + len) != '\0') {
		len++;
	}
	return len;
}

int main() {
	// getting the size of string
	int size = 0;
	scanf("%d\n", &size);
	char* string = (char*)malloc((size + 1) * sizeof(char));

	if (string == NULL) {
		printf("memory allocation failed\n");
		return 1;
	}

	// gettint the string
	for (int i = 0; i < size; i++) {
		scanf(" %c", string + i);
	}
	string[size] = '\0';
	printf("\n");
	
	//printing the string
	for (int i = 0; i < size; ++i) {
		printf("%c", *(string + i));
	}
	
	printf("\n");
	printf("length of string = %d\n", str_length(string));
	return 0;
}
