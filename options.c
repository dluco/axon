#include <assert.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "options.h"
#include "utils.h"

Options *options_new(void)
{
	Options *opts;

	if (!(opts = malloc(sizeof(*opts)))) {
		die("failed to malloc opts\n");
	}

	/* set initial values - don't bother with init function */
	opts->xterm_execute = FALSE;
	opts->version = FALSE;
	opts->login = FALSE;
	opts->hold = FALSE;

	return opts;
}

void options_free(Options *opts)
{
	assert(opts != NULL);

	free(opts);
}

void options_parse(Options *opts, int argc, char *argv[])
{
	GOptionContext *context;
	GError *gerror = NULL;
	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &opts->version, "Print version number", NULL },
		{ "font", 'f', 0, G_OPTION_ARG_STRING, &opts->font, "Select initial terminal font", NULL },
		{ "working-directory", 'd', 0, G_OPTION_ARG_STRING, &opts->work_dir, "Set working directory", NULL },
		{ "execute", 'x', 0, G_OPTION_ARG_STRING, &opts->execute, "Execute command", NULL },
		{ "xterm-execute", 'e', 0, G_OPTION_ARG_NONE, &opts->xterm_execute, "Execute command (last option in the command line)", NULL },
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opts->xterm_args, NULL, NULL },
		{ "login", 'l', 0, G_OPTION_ARG_NONE, &opts->login, "Login shell", NULL },
		{ "title", 't', 0, G_OPTION_ARG_STRING, &opts->title, "Set window title", NULL },
		{ "columns", 'c', 0, G_OPTION_ARG_INT, &opts->columns, "Set number of columns", NULL },
		{ "rows", 'r', 0, G_OPTION_ARG_INT, &opts->rows, "Set number of rows", NULL },
		{ "hold", 'h', 0, G_OPTION_ARG_NONE, &opts->hold, "Hold window after execute command", NULL },
		{ "maximize", 'm', 0, G_OPTION_ARG_NONE, &opts->maximize, "Maximize window", NULL },
		{ "fullscreen", 's', 0, G_OPTION_ARG_NONE, &opts->fullscreen, "Fullscreen mode", NULL },
		{ "geometry", 0, 0, G_OPTION_ARG_STRING, &opts->geometry, "X geometry specification", NULL },
		{ "config-file", 0, 0, G_OPTION_ARG_FILENAME, &opts->config_file, "Use alternate configuration file", NULL },
		{ "output-file", 'o', 0, G_OPTION_ARG_FILENAME, &opts->output_file, "Write terminal contents to file on exit", NULL },
		{ NULL }
	};

	context = g_option_context_new("- terminal emulator");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &argc, &argv, &gerror)) {
		die("%s\n", gerror->message);
	}
	g_option_context_free(context);

	if (opts->version) {
		version();
	}

	/* I don't care if chdir fails - continue anyways */
	if (opts->work_dir) {
		chdir(opts->work_dir);
	}
}