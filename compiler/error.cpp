#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void Error::Do_Error(char *err, int line)
{
	ErrorCount++;
	fprintf(stderr, "%s:", filename);
	if (line > 0)
	{
		fprintf(stderr, "%d:", line);
	}
	fprintf(stderr, " ");
	fprintf(stderr, "error: ");
	fprintf(stderr, "%s", err);
	fprintf(stderr, "\n");
}

void Error::Do_Fatal(char *err, int line)
{
	ErrorCount++;
	fprintf(stderr, "%s:", filename);
	if (line > 0)
	{
		fprintf(stderr, "%d:", line);
	}
	fprintf(stderr, " ");
	fprintf(stderr, "error: ");
	fprintf(stderr, "%s", err);
	fprintf(stderr, "\n");

	exit(-1);
}

