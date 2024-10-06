#include <stdio.h>

int main (){
	int *ptr = NULL;
	printf("Size int: %ld\n", sizeof(int));
	printf("Size float: %ld\n", sizeof(float));
	printf("Size double: %ld\n", sizeof(double));
	printf("Size void: %ld => shouldnt have a size... \n", sizeof(void));
	printf("Size pointer: %ld\n", sizeof(ptr));
	return 0;
}


