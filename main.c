#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "options.h"
#include "utils.h"

/* remove the need for global? */
Config *conf;

static void init(void)
{
	setlocale(LC_ALL, "");
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

int main(int argc, char *argv[])
{
	Terminal *term;
	Options *opts;
	
	opts = options_new();
	options_parse(opts, argc, argv);

	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* perform init of program */
	init();

	/* initialize terminal instance */
	term = terminal_new();
	terminal_init(term);
	terminal_load_config(term, conf);
	terminal_run(term, "command goes here");

	/* run gtk main loop */
	gtk_main();

	/* perform cleanup */
	cleanup();

	return 0;
}
