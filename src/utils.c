#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <glib.h>

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

static void colortable_sub(const gchar *bright, guint start)
{
  guint n;
  guint fg;
  guint bg;

  for (n = start; n <= 37; n++)
    {
      if (n == 28)
        fg = 0;
      else if (n == 29)
        fg = 1;
      else
        fg = n;

      /* blank */
      g_print (" %*s%2dm |",
               2, bright, fg);

      /* without background color */
      g_print ("\e[%s%dm %*s%2dm ",
               bright, fg, 2, bright, fg);

      /* with background color */
      for (bg = 40; bg <= 47; bg++)
        {
          g_print ("\e[%s%d;%dm %*s%2dm ",
                   bright, fg, bg, 2, bright, fg);
        }

      g_print ("\e[0m\n");
    }
}



void colortable(void)
{
  guint bg;

  /* header */
  g_print ("%*s|%*s", 7, "", 7, "");
  for (bg = 40; bg <= 47; bg++)
    g_print ("   %dm ", bg);
  g_print ("\n");

  /* normal */
  colortable_sub ("", 28);

  /* bright */
  colortable_sub ("1;", 30);
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
