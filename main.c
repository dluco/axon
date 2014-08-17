#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "utils.h"

/* remove the need for global? */
Config *conf;

static void init(void)
{
	/* set TERM env-variable? */

	gtk_window_set_default_icon_name("terminal");

	/* create a new Config instance */
	conf = config_new();
	
	/* set default values */
	config_init(conf);
	
	/* load configuration file */
	config_load(conf);
}

static void cleanup(void)
{
	/* save and destroy config */
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
	terminal_load_config(term, conf);
	terminal_run(term, "test");

	/* run gtk main loop */
	gtk_main();

	/* perform cleanup */
	cleanup();

	return 0;
}
