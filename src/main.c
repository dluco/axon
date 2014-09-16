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

int main(int argc, char *argv[])
{
	Config *conf;
	Options *opts;

	/* Initialize GTK+ */
	gtk_init(&argc, &argv);
	
	/* Set name of application */
	g_set_application_name("axon");

	/* Set default window icon for all windows */
	gtk_window_set_default_icon_name("terminal");

	/* Load commandline options */
	opts = options_parse(argc, argv);

	/* Load configuration file */
	conf = config_load_from_file(opts->config_file);

	/* Initialize terminal instance */
	terminal_initialize(conf, opts);

	/* Run GTK main loop */
	gtk_main();

	/* Destroy options and config */
	config_free(conf);

	return 0;
}
