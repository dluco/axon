#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

static void init(void)
{
	/* load config file? */
	return;
}

static void cleanup(void)
{
	printf("in cleanup\n");
	return;
}

static void usage(void)
{
	printf("Usage:\n"
			"  %s [OPTION]\n\n",
			PACKAGE);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}
	
	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* perform init of program */
	init();

	/* initialize terminal instance */
	terminal_new();

	/* run gtk main loop */
	gtk_main();

	/* perform cleanup */
	cleanup();

	return 0;
}
