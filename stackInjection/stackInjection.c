#include <stdio.h>


void win() {
	printf("You win");
	return;
}



int main() {
	printf("Welcome to stack attack by: Jefferson Bland\n");
	printf("try to inject into this program the address of the win function to call it\n");

	printf("perhaps pwntools can help with finding the function offset and exploiting it?");

	return 0;
}
