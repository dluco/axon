#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <vte/vte.h>

#include "config.h"
#include "utils.h"

gint dialog_message_question(GtkWidget *, gchar *, ...);

Config *config_new(void)
{
	Config *conf;

	if (!(conf = malloc(sizeof(*conf)))) {
		die("failure to allocate memory for config\n");
	}

	return conf;
}

/* Initialize Config with default values */
void config_init(Config *conf)
{
	assert(conf != NULL);

	conf->scroll_on_output = SCROLL_ON_OUTPUT;
	conf->scroll_on_keystroke = SCROLL_ON_KEYSTROKE;
	conf->show_scrollbar = SCROLLBAR;
	conf->scrollback_lines = SCROLLBACK_LINES;
	conf->audible_bell = AUDIBLE_BELL;
	conf->visible_bell = VISIBLE_BELL;
	conf->blinking_cursor = BLINKING_CURSOR;
	conf->modified = FALSE;
	conf->modified_externally = FALSE;
}

void config_free(Config *conf)
{
	g_key_file_free(conf->cfg);
	g_free(conf->config_file);
	g_free(conf->font);
}

void config_set_integer(Config *conf, const char *key, int value)
{
	g_key_file_set_integer(conf->cfg, CFG_GROUP, key, value);
	conf->modified = TRUE;
}

void config_set_value(Config *conf, const char *key, const char *value)
{
	g_key_file_set_value(conf->cfg, CFG_GROUP, key, value);
	conf->modified = TRUE;
}

void config_set_boolean(Config *conf, const char *key, gboolean value)
{
	g_key_file_set_boolean(conf->cfg, CFG_GROUP, key, value);
	conf->modified = TRUE;
}

void config_load(Config *conf, char *user_file)
{
	GError *gerror = NULL;
	gchar *tmp = NULL;
	char *config_dir = NULL;

	/* Config file initialization */
	conf->cfg = g_key_file_new();

	if (user_file) {
		/*
		 * A user specified file MUST exist - otherwise, they could give a bogus
		 * file like "/foo/bar" and mess up the root directory (if they had rights)
		 */
		if (g_path_is_absolute(user_file)) {
			/* Absolute path was given */
			conf->config_file = g_strdup(user_file);
		} else {
			/* Relative path to file was given - prepend current directory */
			tmp = g_get_current_dir();
			conf->config_file = g_build_filename(tmp, user_file, NULL);
			g_free(tmp);
		}
		/* Test if user supplied config file actually exists and is not a directory */
		if (!g_file_test(conf->config_file, G_FILE_TEST_IS_REGULAR)) {
			print_err("invalid config file \"%s\"\n", user_file);
		}
	} else {
		config_dir = g_build_filename(g_get_user_config_dir(), "axon", NULL);

		if (!g_file_test(g_get_user_config_dir(), G_FILE_TEST_EXISTS)) {
			/* ~/.config does not exist - create it */
			g_mkdir(g_get_user_config_dir(), 0755);
		}
		if (!g_file_test(config_dir, G_FILE_TEST_EXISTS)) {
			/* Program config dir does not exist - create it */
			g_mkdir(config_dir, 0755);
		}
		conf->config_file = g_build_filename(config_dir, DEFAULT_CONFIG_FILE, NULL);
		
		g_free(config_dir);
	}

	/* Open config file */
	if (!g_key_file_load_from_file(conf->cfg, conf->config_file, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
		/* If file does not exist, then ignore - one will be created */
		if (gerror->code == G_KEY_FILE_ERROR_UNKNOWN_ENCODING ||
			gerror->code == G_KEY_FILE_ERROR_INVALID_VALUE) {
			die("invalid config file format\n");
		}
		g_error_free(gerror);
		gerror = NULL;
	}

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "font", NULL)) {
		config_set_value(conf, "font", DEFAULT_FONT);
	}
	tmp = g_key_file_get_value(conf->cfg, CFG_GROUP, "font", NULL);
	conf->font = g_strdup(tmp);
	free(tmp);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "color_scheme", NULL)) {
		config_set_value(conf, "color_scheme", DEFAULT_COLOR_SCHEME);
	}
	tmp = g_key_file_get_value(conf->cfg, CFG_GROUP, "color_scheme", NULL);
	conf->palette = g_strdup(tmp);
	free(tmp);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "opacity", NULL)) {
		config_set_integer(conf, "opacity", DEFAULT_OPACITY);
	}
	conf->opacity = g_key_file_get_integer(conf->cfg, CFG_GROUP, "opacity", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "title_mode", NULL)) {
		config_set_value(conf, "title_mode", DEFAULT_TITLE_MODE);
	}
	tmp = g_key_file_get_value(conf->cfg, CFG_GROUP, "title_mode", NULL);
	if (strcmp(tmp, "replace") == 0) {
		conf->title_mode = TITLE_MODE_REPLACE;
	} else {
		conf->title_mode = TITLE_MODE_IGNORE;
	}
	free(tmp);
	
	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scroll_on_output", NULL)) {
		config_set_boolean(conf, "scroll_on_output", SCROLL_ON_OUTPUT);
	}
	conf->scroll_on_output = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scroll_on_output", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scroll_on_keystroke", NULL)) {
		config_set_boolean(conf, "scroll_on_keystroke", SCROLL_ON_KEYSTROKE);
	}
	conf->scroll_on_keystroke = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scroll_on_keystroke", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scrollbar", NULL)) {
		config_set_boolean(conf, "scrollbar", SCROLLBAR);
	}
	conf->show_scrollbar = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scrollbar", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scrollback_lines", NULL)) {
		config_set_integer(conf, "scrollback_lines", SCROLLBACK_LINES);
	}
	conf->scrollback_lines = g_key_file_get_integer(conf->cfg, CFG_GROUP, "scrollback_lines", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "allow_bold", NULL)) {
		config_set_boolean(conf, "allow_bold", ALLOW_BOLD);
	}
	conf->allow_bold = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "allow_bold", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "audible_bell", NULL)) {
		config_set_boolean(conf, "audible_bell", AUDIBLE_BELL);
	}
	conf->audible_bell = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "audible_bell", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "visible_bell", NULL)) {
		config_set_boolean(conf, "visible_bell", VISIBLE_BELL);
	}
	conf->visible_bell = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "visible_bell", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "blinking_cursor", NULL)) {
		config_set_boolean(conf, "blinking_cursor", BLINKING_CURSOR);
	}
	conf->blinking_cursor = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "blinking_cursor", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "cursor_type", NULL)) {
		config_set_value(conf, "cursor_type", DEFAULT_CURSOR_TYPE);
	}
	tmp = g_key_file_get_value(conf->cfg, CFG_GROUP, "cursor_type", NULL);
	if (strcmp(tmp, "beam") == 0) {
		conf->cursor_type = VTE_CURSOR_SHAPE_IBEAM;
	} else if (strcmp(tmp , "underline") == 0) {
		conf->cursor_type = VTE_CURSOR_SHAPE_UNDERLINE;
	} else {
		conf->cursor_type = VTE_CURSOR_SHAPE_BLOCK;
	}
	free(tmp);
	
	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "autohide_mouse", NULL)) {
		config_set_boolean(conf, "autohide_mouse", AUTOHIDE_MOUSE);
	}
	conf->autohide_mouse = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "autohide_mouse", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "word_chars", NULL)) {
		config_set_value(conf, "word_chars", DEFAULT_WORD_CHARS);
	}
	conf->word_chars = g_key_file_get_value(conf->cfg, CFG_GROUP, "word_chars", NULL);
}

void config_save(Config *conf, GtkWidget *window)
{
	gint response;
	gboolean overwrite = TRUE;
	GError *gerror = NULL;

	if (!conf->modified) {
		/* No changes made to config */
		return;
	}
	if (conf->modified_externally) {
		/* File changed by another process */
		response = dialog_message_question(window,
				"Configuration has been modified by another process. Overwrite?");

		if (response == GTK_RESPONSE_YES) {
			overwrite = TRUE;
		} else {
			overwrite = FALSE;
		}
	}

	if (overwrite) {
		/* Write contents of cfg to config_file */
		if (!g_key_file_save_to_file(conf->cfg, conf->config_file, &gerror)) {
			die("%s\n", gerror->message);
		}
	}
}
