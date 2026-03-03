#include <stdio.h>

int main() {
	char buffer[256];

	while (1) {
		printf("> ");
		if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
			printf("Repeat you: %s", buffer);
		}
	}
	return 0;
}

