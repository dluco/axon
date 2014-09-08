#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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

void sort_string_array(char **strings)
{
	int n;

	n = g_strv_length(strings);
	qsort(strings, n, sizeof(*strings), string_cmp);
}

int string_cmp(const void *a, const void *b)
{
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	return strcmp(*ia, *ib);
}

void remove_suffix(char *input, char *suffix)
{
	char *tmp;

	/* Find last '.' in string and check if suffix appears after */
	if ((tmp = strrchr(input, '.')) && g_str_has_suffix(tmp, suffix)) {
		*tmp = '\0';
	}
}
