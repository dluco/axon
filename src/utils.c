#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

void die(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");
	
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void print_err(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
}
