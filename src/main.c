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
#include "callback.h"

GSList *terminals = NULL;

static void init(Options *opts, Config *conf)
{
	Terminal *term;
	GFile *cfgfile;
	GFileMonitor *cfgfile_monitor;

	/* Add GFile monitor to control external file changes */
	cfgfile = g_file_new_for_path(conf->config_file);
	cfgfile_monitor = g_file_monitor_file(cfgfile, 0, NULL, NULL);
	g_signal_connect_swapped(G_OBJECT(cfgfile_monitor), "changed",
			G_CALLBACK(config_file_changed), conf);

	/* FIXME: set TERM env-variable? */

	/* Set name of application */
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
}

static void cleanup(Options *opts, Config *conf)
{
	/* Destroy options and config */
	options_free(opts);
	config_free(conf);
}

int main(int argc, char *argv[])
{
	Options *opts;
	Config *conf;
	
	setlocale(LC_ALL, "");

	/* Load commandline options */
	opts = options_new();
	options_parse(opts, argc, argv);

	/* Load configuration file */
	conf = config_new();
	config_init(conf);
	config_load(conf, opts->config_file);

	/* Perform initialization */
	init(opts, conf);
	
	/* Run GTK main loop */
	gtk_main();

	/* Perform cleanup */
	cleanup(opts, conf);

	return 0;
}
