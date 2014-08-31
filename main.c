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

static void init(Options *opts, Config *conf)
{
	setlocale(LC_ALL, "");

	/* set TERM env-variable? */

	config_init(conf);
	
	/* Load configuration file */
	config_load(conf, opts->config_file);

	/* Set default window icon for all windows */
	gtk_window_set_default_icon_name("terminal");
}

static void cleanup(Options *opts, Config *conf)
{
	options_free(opts);
	
	/* Save and destroy config */
	config_save(conf);
	config_free(conf);
}

int main(int argc, char *argv[])
{
	Options *opts;
	Config *conf;
	Terminal *term;
	
	/* Load commandline options */
	opts = options_new();
	options_parse(opts, argc, argv);

	/* Initialize gtk+ */
	/* FIXME: move to options_parse? */
	gtk_init(&argc, &argv);

	conf = config_new();
	
	/* Perform init of program */
	init(opts, conf);

	/* Initialize terminal instance */
	term = terminal_new();
	terminal_init(term);
	terminal_load_config(term, conf);
	terminal_load_options(term, opts);
	terminal_run(term, "command goes here");

	/* Run gtk main loop */
	gtk_main();

	/* Perform cleanup */
	cleanup(opts, conf);

	return 0;
}
