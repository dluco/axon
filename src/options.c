#include <stdlib.h>
#include <gtk/gtk.h>

#include "options.h"
#include "utils.h"

Options *options_parse(int argc, char *argv[])
{
	Options *opts;
	GOptionContext *context;
	GError *gerror = NULL;
	int i, n;
	int t_argc;
	char **t_argv;
	gboolean match = FALSE;

	/* Allocate Options */
	opts = g_new0(Options, 1);

	GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &opts->version, "Print version number", NULL },
		{ "config", 'c', 0, G_OPTION_ARG_FILENAME, &opts->config_file, "Load a terminal configuration file", "FILE" },
		{ "working-directory", 'd', 0, G_OPTION_ARG_STRING, &opts->work_dir, "Set the working directory", "DIR" },
		{ "command", 'x', 0, G_OPTION_ARG_STRING, &opts->command, "Execute command", "COMMAND" },
		{ "execute", 'e', 0, G_OPTION_ARG_NONE, &opts->execute, "Execute command (last option in the command line)", NULL },
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &opts->execute_args, NULL, NULL },
		{ "login", 'l', 0, G_OPTION_ARG_NONE, &opts->login, "Login shell", NULL },
		{ "title", 't', 0, G_OPTION_ARG_STRING, &opts->title, "Set window title", "TITLE" },
		{ "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &opts->fullscreen, "Fullscreen mode", NULL },
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
	t_argc = argc;
	t_argv = calloc(argc + 1, sizeof(*t_argv));
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

	g_strfreev(t_argv);

	/* Print version info and exit */
	if (opts->version) {
		printf("axon - %s, 2014 David Luco <dluco11@gmail.com>\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	/* A working directory must always be provided */
	if (!opts->work_dir) {
		opts->work_dir = g_get_current_dir();
	}

	return opts;
}
