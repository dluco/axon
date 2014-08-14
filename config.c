#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "config.h"
#include "utils.h"

Config *config_new(void)
{
	Config *conf;

	if (!(conf = malloc(sizeof(*conf)))) {
		die("failure to malloc config\n");
	}

	return conf;
}

void config_load(Config *conf)
{
	GError *gerror = NULL;
	gchar *tmp = NULL;
	char *config_dir = NULL;

	/* Config file initialization */
	conf->cfg = g_key_file_new();
	conf->modified = FALSE;

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
	conf->config_file = g_build_filename(config_dir, DEFAULT_CONFIG_FILE, NULL);

	/* Open config file */
	if (!(g_key_file_load_from_file(conf->cfg, conf->config_file, G_KEY_FILE_KEEP_COMMENTS, &gerror))) {
		/* if file does not exist, then ignore - one will be created */
		if (gerror->code == G_KEY_FILE_ERROR_UNKNOWN_ENCODING ||
			gerror->code == G_KEY_FILE_ERROR_INVALID_VALUE) {
			die("invalid config file format\n");
		}
		g_error_free(gerror);
		gerror = NULL;
	}
		
	g_free(config_dir);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "font", NULL)) {
		g_key_file_set_value(conf->cfg, CFG_GROUP, "font", DEFAULT_FONT);
		conf->modified = TRUE;
	}
	tmp = g_key_file_get_value(conf->cfg, CFG_GROUP, "font", NULL);
	conf->font = g_strdup(tmp);
	free(tmp);
	
	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scroll_on_output", NULL)) {
		g_key_file_set_boolean(conf->cfg, CFG_GROUP, "scroll_on_output", SCROLL_ON_OUTPUT);
		conf->modified = TRUE;
	}
	conf->scroll_on_output = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scroll_on_output", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scroll_on_keystroke", NULL)) {
		g_key_file_set_boolean(conf->cfg, CFG_GROUP, "scroll_on_keystroke", SCROLL_ON_KEYSTROKE);
		conf->modified = TRUE;
	}
	conf->scroll_on_keystroke = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scroll_on_keystroke", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scrollbar", NULL)) {
		g_key_file_set_boolean(conf->cfg, CFG_GROUP, "scrollbar", SCROLLBAR);
		conf->modified = TRUE;
	}
	conf->show_scrollbar = g_key_file_get_boolean(conf->cfg, CFG_GROUP, "scrollbar", NULL);

	if (!g_key_file_has_key(conf->cfg, CFG_GROUP, "scrollback_lines", NULL)) {
		g_key_file_set_integer(conf->cfg, CFG_GROUP, "scrollback_lines", SCROLLBACK_LINES);
		conf->modified = TRUE;
	}
	conf->scrollback_lines = g_key_file_get_integer(conf->cfg, CFG_GROUP, "scrollback_lines", NULL);
}

void config_save(Config *conf)
{
	GError *gerror = NULL;
	gsize len = 0;
	gchar *cfgdata;
	GIOChannel *cfgfile;
	GIOStatus status;

	cfgdata = g_key_file_to_data(conf->cfg, &len, &gerror);
	if (!cfgdata) {
		die("%s\n", gerror->message);
	}

	if (conf->modified) {
		cfgfile = g_io_channel_new_file(conf->config_file, "w", &gerror);
		if (!cfgfile) {
			die("%s\n", gerror->message);
		}

		/* FIXME: if the number of chars written is not "len", something happened.
		 * Check for errors appropriately...*/
		status = g_io_channel_write_chars(cfgfile, cfgdata, len, NULL, &gerror);
		if (status != G_IO_STATUS_NORMAL) {
			// FIXME: we should deal with temporary failures (G_IO_STATUS_AGAIN)
			die("%s\n", gerror->message);
		}
		g_io_channel_shutdown(cfgfile, TRUE, &gerror);
		g_io_channel_unref(cfgfile);
	}
}

void config_destroy(Config *conf)
{
	g_key_file_free(conf->cfg);
	free(conf->config_file);
	free(conf->font);
}
