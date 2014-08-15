#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", PACKAGE);
	
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void print_err(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "%s", PACKAGE);

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
}
