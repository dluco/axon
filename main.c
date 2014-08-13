#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "config.h"
#include "terminal.h"
#include "utils.h"

GKeyFile *cfg;
const char *cfg_group = PACKAGE;
char *config_file;

int scroll_on_output;;
int scroll_on_keystroke;
int show_scrollbar;
int scrollback_lines;

static void init(void)
{
	GError *gerror = NULL;
	gchar *tmp = NULL;
	char *config_dir = NULL;

	/* set global variables */
	scroll_on_output = FALSE;
	scroll_on_keystroke = TRUE;
	show_scrollbar = TRUE;
	scrollback_lines = -1;
	
	/* set TERM env-variable? */

	/* Config file initialization */
	cfg = g_key_file_new();

	config_dir = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	if (!g_file_test(g_get_user_config_dir(), G_FILE_TEST_EXISTS)) {
		/* ~/.config does not exist - create it */
		g_mkdir(g_get_user_config_dir(), 0755);
	}
	if (!g_file_test(config_dir, G_FILE_TEST_EXISTS)) {
		/* program config dir does not exist - create it */
		g_mkdir(config_dir, 0755);
	}
	/* TODO: allow user to specify config file */
	config_file = g_build_filename(config_dir, PACKAGE "rc", NULL);

	/* Open config file */
	if (!(g_key_file_load_from_file(cfg, config_file, G_KEY_FILE_KEEP_COMMENTS, &gerror))) {
		/* if file does not exist, then ignore - one will be created */
		if (gerror->code == G_KEY_FILE_ERROR_UNKNOWN_ENCODING ||
			gerror->code == G_KEY_FILE_ERROR_INVALID_VALUE) {
			die("invalid config file format\n");
		}
	}
		
	g_free(config_dir);

	if (!g_key_file_has_key(cfg, cfg_group, "font", NULL)) {
		g_key_file_set_value(cfg, cfg_group, "font", "Ubuntu Mono,monospace 13");
	}
	tmp = g_key_file_get_value(cfg, cfg_group, "font", NULL);
	free(tmp);

	
	if (!g_key_file_has_key(cfg, cfg_group, "scrollbar", NULL)) {
		g_key_file_set_boolean(cfg, cfg_group, "scrollbar", TRUE);
	}
	show_scrollbar = g_key_file_get_boolean(cfg, cfg_group, "scrollbar", NULL);

	return;
}

static void cleanup(void)
{
	/* TODO: move to destroy callback */
	g_key_file_free(cfg);

	free(config_file);
}

static void usage(void)
{
	printf("Usage:\n"
			"  %s [OPTION]\n\n",
			PACKAGE);
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}
	
	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* perform init of program */
	init();

	/* initialize terminal instance */
	terminal_new();

	/* run gtk main loop */
	gtk_main();

	/* perform cleanup */
	cleanup();

	return 0;
}
