#include <stdio.h>
#include <string.h>

//we need to define MAX at compile time since its needed to run the main
#define MAX 100

//important when initialising arrays:
//either specify the size of the array and leave value open
//or give array a value and dont specify the size
char first[] = "empty firstname";
char second[] = "empty secondname";
int year = 0;
char str[MAX];
char name[MAX];
char *input = "first second 0";


void reverseString(char input[]);

int main(){
        printf("enter your first name: ");
	scanf("%s", first);     //scanf("%s", &first[0]); would do the same, strings dont need the & sign
	printf("enter your second name: ");
	scanf("%s", second);
	printf("enter your birth year: ");
	scanf("%d", &year);     //ints do need the &sign, same as with chars
	printf("first: %s - second: %s - birth year: %i \n", first, second, year);
	
	//test out reversing lower to upper and the other way around
	reverseString(second);
	printf("reversed upper and lower: %s\n", str);
	
	//test out the strcmp function
	//we expect to get -32 if the first char of string second is Capital
	//we expect to get 32 if the first char of string second is lower case
        //strcmp stops at the first mismatch... (always the first char for this example)
	printf("strcmp(): %d \n", strcmp(second, str));
	
	//test out strcpy
	strncpy(name, first, sizeof(name)-1); //copy as much as possible, but leave space for the str end char
	name[strlen(name)] = '\0';
	printf("strncpy(): %s \n", name);
	
	strncat(name, second, sizeof(name) - strlen(name) - 1);
	printf("strncat(): %s \n", name);
	
	//test out snprintf()
	//concatenates in a certain format
	snprintf(name, sizeof(name), "%s %s %d", first, second, year);
	printf("snprintf(): %s \n", name);
	
	//test out sscanf()
	//sscanf extracts data from name, following a certain format and saves that data into seperate vars
	int amount = sscanf(name, "%s %s %d", first, second, &year);
	//printf("%d \n", amount);
	if (amount == 3){
	        printf("sscanf(): \n");
		printf("first: %s\n", first);
		printf("second: %s\n", second);
		printf("birthyear: %d\n", year);
	} else {
		printf("failed scanning \n");
	}
	
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
