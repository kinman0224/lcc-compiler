#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "ir.h"
#include "compiler.h"

using namespace std;

#define DEBUG_PRINT(msg) {printf("lcc: %s\n", msg);}

int main(int argc, char **argv)
{
	char *input = NULL, *output = NULL;

	for (int i = 1; i < argc; i++) {

		if (strncmp(argv[i], "-", 1) != 0 && strncmp(argv[i], "--", 2) != 0) {
			input = argv[i];
		}
		else
		if (strcmp(argv[i], "--help") == 0) {
			printf("Usage: compiler [option] file..\n");
			printf("Options:\n");
			printf("  --help					Display this information\n");
			printf("  -S						Compile only\n");
			printf("  -o <file>					Place the output into <file>\n");
			printf("\n");

			return 0;
		}
		else
		if (strcmp(argv[i], "-S") == 0) {

		}
		else
		if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 == argc) {
				DEBUG_PRINT("argument to `-o' is missing");
			}
			else {
				output = argv[i + 1];
				i++;
			}
		}
		else {
			char tmp[256];
			sprintf(tmp, "unrecognized option '%s'", argv[i]);
			DEBUG_PRINT(tmp);
		}
	}

	if (input == NULL) {
		DEBUG_PRINT("no input files\n");
		return 0;
	}

	Compiler compiler;
	compiler.Compilation(input, output);


	//system("pause");
	return 0;
}