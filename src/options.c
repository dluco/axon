#include <assert.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "options.h"
#include "utils.h"

Options *options_new(void)
{
	Options *opts;

	if (!(opts = malloc(sizeof(*opts)))) {
		die("failure to allocate memory for options\n");
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
	int i;
	int n;
	int t_argc;
	char **t_argv;
	gboolean match = FALSE;

	GOptionContext *context;
	GError *gerror = NULL;
	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &opts->version, "Print version number", NULL },
		{ "color-table", 0, 0, G_OPTION_ARG_NONE, &opts->colortable, "Echo color codes", NULL },
		{ "config", 'c', 0, G_OPTION_ARG_FILENAME, &opts->config_file, "Load a configuration file", "FILE" },
		{ "output", 'o', 0, G_OPTION_ARG_FILENAME, &opts->output_file, "Write terminal contents to file on exit", "FILE" },
		{ "working-directory", 'd', 0, G_OPTION_ARG_STRING, &opts->work_dir, "Set the working directory", "DIR" },
		{ "execute", 'x', 0, G_OPTION_ARG_STRING, &opts->execute, "Execute command", NULL },
		{ "xterm-execute", 'e', 0, G_OPTION_ARG_NONE, &opts->xterm_execute, "Execute command (last option in the command line)", NULL },
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opts->xterm_args, NULL, NULL },
		{ "login", 'l', 0, G_OPTION_ARG_NONE, &opts->login, "Login shell", NULL },
		{ "hold", 'H', 0, G_OPTION_ARG_NONE, &opts->hold, "Hold window after execute command", NULL },
		{ "font", 'f', 0, G_OPTION_ARG_STRING, &opts->font, "Select initial terminal font", "FONT" },
		{ "title", 't', 0, G_OPTION_ARG_STRING, &opts->title, "Set window title", "TITLE" },
		{ "fullscreen", 's', 0, G_OPTION_ARG_NONE, &opts->fullscreen, "Fullscreen mode", NULL },
		{ "maximize", 'm', 0, G_OPTION_ARG_NONE, &opts->maximize, "Maximize window", NULL },
		{ "geometry", 'g', 0, G_OPTION_ARG_STRING, &opts->geometry, "X geometry specification", "GEOMETRY" },
		{ NULL }
	};

	/* 
	 * Rewrite argv to include a "--" after the -e argument. This is necessary to make
	 * sure GOption doesn't grab any arguments meant for the command being called.
	 * The "match" flag is used so that commands such as:
	 * $ axon -e xterm -e ranger
	 * can be run correctly. However, as soon as the "-e" option is given,
	 * ALL other options afterwards are considered part of the execute command. Therefore,
	 * the "-e" option should ONLY be given after all other options.
	 */
	t_argv = calloc(argc + 1, sizeof(*t_argv));
	t_argc = argc;
	n = 0;

	for (i = 0; i < argc; i++) {
		if (g_strcmp0(argv[i], "-e") == 0 && !match) {
			t_argv[n] = "-e";
			n++;
			t_argv[n] = "--";
			t_argc = argc + 1;
			match = TRUE;
		} else {
			t_argv[n] = g_strdup(argv[i]);
		}
		n++;
	}

	context = g_option_context_new("- terminal emulator");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, &t_argc, &t_argv, &gerror)) {
		die("%s\n", gerror->message);
	}
	g_option_context_free(context);

	/* Initialize GTK+ */
	gtk_init(&t_argc, &t_argv);

	g_strfreev(t_argv);

	/* Print version info and exit */
	if (opts->version) {
		version();
		exit(EXIT_SUCCESS);
	}

	/* Print out color-table and exit */
	if (opts->colortable) {
		colortable();
		exit(EXIT_SUCCESS);
	}

	/* I don't care if chdir fails - continue anyways */
	if (opts->work_dir) {
		chdir(opts->work_dir);
	}
}
