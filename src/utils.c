#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "utils.h"

void version(void)
{
	printf("axon - %s, 2014 David Luco <dluco11@gmail.com>\n", VERSION);
	exit(EXIT_SUCCESS);
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

void remove_suffix(char *input, char *suffix)
{
	char *tmp;

	if ((tmp = strrchr(input, '.')) && g_str_has_suffix(tmp, suffix)) {
		*tmp = '\0';
	}
}
