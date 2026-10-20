#include <stdio.h>

void never_called() {
	printf("[*] wow how did you manage to call this?\n");
	return;
}

void will_called() {
	printf("[*] you!\n");
	return ;
}

int main() {
	printf("[*] you will run this bunaty? Please dont!\n");
	

	will_called();

	return 1;
}

