#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "config.h"
#include "terminal.h"
#include "utils.h"

Config *conf;

static void init(void)
{
	/* set TERM env-variable? */

	/* create a new Config instance */
	conf = config_new();
	
	/* set default values */
	conf->scroll_on_output = FALSE;
	conf->scroll_on_keystroke = TRUE;
	conf->show_scrollbar = TRUE;
	conf->scrollback_lines = -1;
	
	config_load(conf);
}

static void cleanup(void)
{
	config_save(conf);

	config_destroy(conf);
}

static void usage(void)
{
	printf("Usage:\n"
			"  %s [OPTION]\n\n",
			PACKAGE);
}

int main(int argc, char *argv[])
{
	Terminal *term;

	if (argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}
	
	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* perform init of program */
	init();

	/* initialize terminal instance */
	term = terminal_new();
	terminal_init(term);

	/* run gtk main loop */
	gtk_main();

	/* perform cleanup */
	cleanup();

	return 0;
}
