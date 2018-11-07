

#include <stdio.h>

#define BUFFER_SIZE 65535









int main(int argc, char ** argv) {
	char array[BUFFER_SIZE];
	char * ptr = array;
	
	char code_b[65535];
	char* code = code_b;
	char* stack[1024];
	unsigned int stack_top = 0;
	unsigned int skip = 0;
	
	FILE * f = fopen(argv[1], "r");
	
	unsigned int len = fread(code_b, 1, sizeof(code_b)-1, f);
	
	code_b[len] = '\0';
	
	fclose(f);
	
	for(unsigned int i=0; i<BUFFER_SIZE; ++i) {
		array[i] = 0;
	}
	
	while(*code != '\0') {
		if(skip) {
			if(*code == ']') --skip;
			else if(*code == '[') ++skip;
			++code;
			continue;
		}
		switch(*code) {
			case '>': {
				++ptr;
				break;
			}
			case '<': {
				--ptr;
				break;
			}
			case '+': {
				++(*ptr);
				break;
			}
			case '-': {
				--(*ptr);
				break;
			}
			case '.': {
				putchar(*ptr);
				break;
			}
			case ',': {
				*ptr=getchar();
				break;
			}
			case '[': {
				if(*ptr) stack[++stack_top] = code;
				else ++skip;
				break;
			}
			case ']': {
				if(*ptr) code = stack[stack_top];
				else --stack_top;
				break;
			}
			default:
				break;
		}
		++code;
	}
	return 0;
}




