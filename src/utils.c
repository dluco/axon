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

static void colortable_sub(const char *bright, unsigned int start)
{
  unsigned int n;
  unsigned int fg;
  unsigned int bg;

  for (n = start; n <= 37; n++)
    {
      if (n == 28)
        fg = 0;
      else if (n == 29)
        fg = 1;
      else
        fg = n;

      /* blank */
      printf(" %*s%2dm |",
               2, bright, fg);

      /* without background color */
      printf("\e[%s%dm %*s%2dm ",
               bright, fg, 2, bright, fg);

      /* with background color */
      for (bg = 40; bg <= 47; bg++)
        {
          printf("\e[%s%d;%dm %*s%2dm ",
                   bright, fg, bg, 2, bright, fg);
        }

      printf("\e[0m\n");
    }
}

void colortable(void)
{
  unsigned int bg;

  /* header */
  printf("%*s|%*s", 7, "", 7, "");
  for (bg = 40; bg <= 47; bg++)
    printf("   %dm ", bg);
  printf("\n");

  /* normal */
  colortable_sub ("", 28);

  /* bright */
  colortable_sub ("1;", 30);
}

unsigned int strv_length(char **str_array)
{
	unsigned int i;

	if (str_array == NULL) return 0;

	for (i = 0; str_array[i]; i++);

	return i;
}

static int str_cmp(const void *a, const void *b)
{
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	return strcmp(*ia, *ib);
}

void strv_sort(char **str_array)
{
	int n;

	n = strv_length(str_array);

	qsort(str_array, n, sizeof(*str_array), str_cmp);
}

int str_has_suffix(const char *str, const char *suffix)
{
	int str_len;
	int suffix_len;

	if (str == NULL || suffix == NULL) {
		return 0;
	}

	str_len = strlen(str);
	suffix_len = strlen(suffix);

	if (str_len < suffix_len) {
		return 0;
	}

	return strcmp(str + str_len - suffix_len, suffix) == 0;
}

void str_remove_suffix(char *input, char *suffix)
{
	char *tmp;

	if (str_has_suffix(input, suffix)) {
		/* nul-terminate string at the last dot */
		tmp = strrchr(input, '.');
		*tmp = '\0';
	}
}
