#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "options.h"
#include "config.h"
#include "terminal.h"
#include "utils.h"

GSList *terminals = NULL;

int main(int argc, char *argv[])
{
	Options *opts;
	Config *conf;
	Terminal *term;
	
	setlocale(LC_ALL, "");

	/* Load commandline options */
	opts = options_new();
	options_parse(opts, argc, argv);

	conf = config_new();
	config_init(conf);
	/* Load configuration file */
	config_load(conf, opts->config_file);
	
	/* FIXME: set TERM env-variable? */

	g_set_application_name("axon");
	/* Set default window icon for all windows */
	gtk_window_set_default_icon_name("terminal");

	/* Initialize terminal instance */
	term = terminal_new();
	terminal_init(term);
	/* Load config THEN options - options get the last say */
	terminal_load_config(term, conf);
	terminal_load_options(term, opts);
	terminal_run(term, NULL);

	terminal_show(term);

	/* Run gtk main loop */
	gtk_main();

	return 0;
}
