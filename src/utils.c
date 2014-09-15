#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"

void version(void)
{
	printf("axon - %s, 2014 David Luco <dluco11@gmail.com>\n", VERSION);
}

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
