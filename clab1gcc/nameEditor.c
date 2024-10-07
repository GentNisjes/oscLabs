#include <stdio.h>
#include <string.h>

//we need to define MAX at compile time since its needed to run the main
#define MAX 20

//int MAX = 25;
char first[MAX];
char second[MAX];

void reverseString(char input[], char reversedStr[] );

int main(){
	scanf("%s", first);
	scanf("%s", second);
	printf("this is %s - %s \n", second, first);
	
	char reversed[MAX];
	reverseString(second, reversed);
	printf("reversed: %s\n", reversed);
	return 0;
}

//should still replace MAX with sizeof() in following
void reverseString(char input[], char reversedStr[] ){
	int i;
	int len = strlen(input);
	for (i = 0; i < len; i++) {
		int ascii = (int)input[i];	//get the ascii value of the char by casting it to an int
		printf("%c \n", input[i]);
		if ('A' >= ascii && ascii <= 'Z'){
			ascii += 32;
		} else if ('a' >= ascii && ascii <= 'z'){
			ascii -= 32;
		}
		reversedStr[i] = (char)ascii;
        }
	//reversedStr[len] += "\0";
	printf("%s\n", reversedStr);
	reversedStr;
}
