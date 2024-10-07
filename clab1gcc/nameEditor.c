#include <stdio.h>
#include <string.h>

//we need to define MAX at compile time since its needed to run the main
#define MAX 20

//int MAX = 25;
char first[MAX];
char second[MAX];
char str[MAX];

void reverseString(char input[]);

int main(){
	scanf("%s", first);
	scanf("%s", second);
	printf("first: %s - second: %s \n", second, first);
	
	reverseString(second);
	printf("reversed upper and lower: %s\n", str);
	
	//test out the strcmp function
	//we expect to get -32 if the first char of string second is Capital
	//we expect to get 32 if the first char of string second is lower case
        //strcmp stops at the first mismatch... (always the first char for this example)
	printf("strcmp(): %d \n", strcmp(second, str));
	
	return 0;
}

//should still replace MAX with sizeof() in following
void reverseString(char input[]){
	int i;
	int len = strlen(input);
	//alternative for for loop middle condition is: s[i]!='\0' => no string.h needed
	for (i = 0; i < len; i++) {
		printf("%c \n", input[i]);
		if (input[i] >= 'A' && input[i] <= 'Z'){
			str[i] = input[i] + 32;
		} else if (input[i] >= 'a' && input[i] <= 'z'){
			str[i] = input[i] - 32;
		}
        }
	printf("%s\n", str);
}
