#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "menu.h"
#include "utils.h"

Terminal *terminal_new(void)
{
	Terminal *term;

	if (!(term = malloc(sizeof(*term)))) {
		die("failure to allocate memory for terminal\n");
	}

	return term;
}

void terminal_init(Terminal *term)
{
	/* initialize ui elements */
	term->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	term->menu = gtk_menu_new();
	term->hbox = gtk_hbox_new(FALSE, 0);
	term->vte = vte_terminal_new();
	term->scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(term->vte)));

	/* set state variables */
	term->fullscreen = FALSE;

	/* setup */
	gtk_window_set_icon_name(GTK_WINDOW(term->window), "xterm");

	menu_popup_init(term->menu, term);
	
	/* arranging */
	gtk_box_pack_start(GTK_BOX(term->hbox), term->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(term->hbox), term->scrollbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(term->window), term->hbox);
	
	/* signal setup */
	g_signal_connect(G_OBJECT(term->window), "delete-event",
			G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(term->window), "destroy",
			G_CALLBACK(destroy_window), term);
	g_signal_connect(G_OBJECT(term->window), "key-press-event",
			G_CALLBACK(key_press), term);

	/* Connect to the "char-size-changed" signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(term->vte), 0, 0, term->window);
	g_signal_connect(G_OBJECT(term->vte), "char-size-changed",
			G_CALLBACK(char_size_changed), term->window);
	g_signal_connect(G_OBJECT(term->vte), "realize",
			G_CALLBACK(char_size_realized), term->window);

	g_signal_connect(G_OBJECT(term->vte), "button-press-event",
			G_CALLBACK(button_press), term);
	g_signal_connect(G_OBJECT(term->vte), "child-exited",
			G_CALLBACK(child_exited), term);
	g_signal_connect(G_OBJECT(term->vte), "eof",
			G_CALLBACK(eof), term);
	g_signal_connect(G_OBJECT(term->vte), "window-title-changed",
			G_CALLBACK(set_title), term->window);
	g_signal_connect(G_OBJECT(term->vte), "increase-font-size",
			G_CALLBACK(increase_font_size), term->window);
	g_signal_connect(G_OBJECT(term->vte), "decrease-font-size",
			G_CALLBACK(decrease_font_size), term->window);
}

void terminal_load_config(Terminal *term, Config *conf)
{
	GRegex *regex;
	GError *gerror = NULL;
	int id;

	term->conf = conf;

	vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), conf->font);
	
	terminal_load_color_scheme(term, conf->color_scheme);

	/* Scroll-related stuff */
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(term->vte), conf->scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(term->vte), conf->scroll_on_keystroke);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), conf->scrollback_lines);

	/* set the annoying bells */
	vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), conf->audible_bell);
	vte_terminal_set_visible_bell(VTE_TERMINAL(term->vte), conf->visible_bell);
	
	/* disable the stupid blinking cursor... */
	vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte),
			(conf->blinking_cursor) ?
				VTE_CURSOR_BLINK_ON :
				VTE_CURSOR_BLINK_OFF);
	vte_terminal_set_cursor_shape(VTE_TERMINAL(term->vte), conf->cursor_type);
	vte_terminal_set_mouse_autohide(VTE_TERMINAL(term->vte), conf->autohide_mouse);
	vte_terminal_set_word_chars(VTE_TERMINAL(term->vte), conf->word_chars);

	/* build the url regex */
	regex = g_regex_new(HTTP_REGEX, G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY, &gerror);
	if (gerror) {
		print_err("%s\n", gerror->message);
		g_error_free(gerror);
		return;
	}
	id = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), regex, 0);
	/* release the regex owned by vte now */
	g_regex_unref(regex);
	vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), id, GDK_HAND1);
}

void terminal_load_options(Terminal *term, Options *opts)
{
	char *geometry;

	term->opts = opts;

	if (opts->title) {
		/* TODO: allow for title "modes" - append, replace (default), ignore */
		gtk_window_set_title(GTK_WINDOW(term->window), opts->title);
	}
	
	if (opts->font) {
		term->conf->font = g_strdup(opts->font);
	}

	/* mutually exclusive options, obviously */
	if (opts->fullscreen) {
		/* Correctly set state of menu item */
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(term->fullscreen_item), TRUE);
	} else if (opts->maximize) {
		gtk_window_maximize(GTK_WINDOW(term->window));
	}

	/* Apply geometry */
	gtk_widget_realize(term->vte);
	geometry = (opts->geometry) ? g_strdup(opts->geometry) :
			g_strdup_printf("%dx%d", DEFAULT_COLUMNS - 1, DEFAULT_ROWS);
	if (!gtk_window_parse_geometry(GTK_WINDOW(term->window), geometry)) {
		print_err("invalid geometry format\n");
	}
	g_free(geometry);
}

void terminal_load_color_scheme(Terminal *term, const char *color_scheme)
{
	GKeyFile *cfg;
	GdkColor fg;
	GdkColor bg;
	GdkColor cursor;
	GdkColor palette[PALETTE_SIZE];
	gboolean has_fg = FALSE;
	gboolean has_bg = FALSE;
	gboolean has_cursor = FALSE;
	gboolean valid_palette = FALSE;
	gchar *scheme_file;
	gchar **palette_colors;
	gchar *tmp;
	int n = 0;
	GError *gerror = NULL;

	/* Config file initialization */
	cfg = g_key_file_new();

	tmp = g_strdup_printf("%s.%s", color_scheme, "theme");
	scheme_file = g_build_filename(DATADIR, "axon", "colorschemes", tmp, NULL);
	g_free(tmp);

	/* Open config file */
	if (!g_key_file_load_from_file(cfg, scheme_file, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
		/* color scheme file not found - default terminal colors will be used */
		print_err("color scheme \"%s\": %s\n", color_scheme, gerror->message);
		g_error_free(gerror);
		gerror = NULL;
	} else {
		/* Begin parsing scheme file */
		if ((tmp = g_key_file_get_value(cfg, "scheme", "color_foreground", NULL))) {
			has_fg = gdk_color_parse(tmp, &fg);
			g_free(tmp);
		}

		if ((tmp = g_key_file_get_value(cfg, "scheme", "color_background", NULL))) {
			has_bg = gdk_color_parse(tmp, &bg);
			g_free(tmp);
		}

		if ((tmp = g_key_file_get_value(cfg, "scheme", "color_cursor", NULL))) {
			has_cursor = gdk_color_parse(tmp, &cursor);
			g_free(tmp);
		}

		if ((palette_colors = g_key_file_get_string_list(cfg, "scheme", "color_palette", NULL, NULL))) {
			if (palette_colors) {
				for (n = 0; palette_colors[n] != NULL && n < PALETTE_SIZE; n++) {
					if (!gdk_color_parse(palette_colors[n], palette + n)) {
						print_err("unable to parse color \"%s\"", palette_colors[n]);
						break; /* Cause valid_palette to fail */
					}
				}
			}
			g_strfreev(palette_colors);
			valid_palette = (n == PALETTE_SIZE);
		}

		/* Apply color palette to terminal */
		if (valid_palette) {
			vte_terminal_set_colors(VTE_TERMINAL(term->vte),
					has_fg ? &fg : NULL, has_bg ? &bg : NULL,
					palette, PALETTE_SIZE);
		} else {
			/* No palette - set default colors */
			vte_terminal_set_default_colors(VTE_TERMINAL(term->vte));
			/* Apply fore/background colors if present */
			vte_terminal_set_color_foreground(VTE_TERMINAL(term->vte), has_fg ? &fg : NULL);
			vte_terminal_set_color_background(VTE_TERMINAL(term->vte), has_bg ? &bg : NULL);
		}

		/* Apply cursor color */
		vte_terminal_set_color_cursor(VTE_TERMINAL(term->vte), has_cursor ? &cursor : NULL);
	}

	g_key_file_free(cfg);
	g_free(scheme_file);
}

void terminal_run(Terminal *term)
{
	int cmd_argc = 0;
	char **cmd_argv;
	char *cmd_joined;
	char *fork_argv[3];
	GError *gerror = NULL;
	char *path;

	if (term->opts->execute || term->opts->xterm_execute) {
		if (term->opts->execute) {
			/* -x option */
			if (!g_shell_parse_argv(term->opts->execute, &cmd_argc, &cmd_argv, &gerror)) {
				die("%s\n", gerror->message);
			}
		} else {
			/* -e option - last in commandline, scoops up rest of arguments */
			if (term->opts->xterm_args) {
				cmd_joined = g_strjoinv(" ", term->opts->xterm_args);
				if (!g_shell_parse_argv(cmd_joined, &cmd_argc, &cmd_argv, &gerror)) {
					die("%s\n", gerror->message);
				}
				g_free(cmd_joined);
			}
		}

		/* Check for a valid command */
		if (cmd_argc > 0) {
			path = g_find_program_in_path(cmd_argv[0]);
			if (path) {
				if (!vte_terminal_fork_command_full(VTE_TERMINAL(term->vte), VTE_PTY_DEFAULT,
						NULL, cmd_argv, NULL, G_SPAWN_SEARCH_PATH,
						NULL, NULL, &term->pid, &gerror)) {
					/* Non-fatal error */
					print_err("%s\n", gerror->message);
				}
			} else {
				/* Fatal error - otherwise it gets tricky */
				die("%s: command not found\n", cmd_argv[0]);
			}
			g_free(path);
			g_strfreev(cmd_argv);
			//g_strfreev(term->opts->xterm_execute_args);
		}
	} else { 
		/* No execute option */
		if (term->opts->hold) { /* FIXME: is this necessary? */
			print_err("hold option given without any command\n");
			term->opts->hold = FALSE;
		}

		/* 
		 * Set up args in preparation for fork. Real argv vector starts at
		 * argv[1] because we're using G_SPAWN_FILE_AND_ARGV_ZERO to be
		 * able to launch login shells.
		 */
		fork_argv[0] = g_strdup(g_getenv("SHELL"));
		if (term->opts->login) {
			fork_argv[1] = g_strdup_printf("-%s", g_getenv("SHELL"));
		} else {
			fork_argv[1] = g_strdup(g_getenv("SHELL"));
		}
		fork_argv[2] = NULL;

		vte_terminal_fork_command_full(VTE_TERMINAL(term->vte), VTE_PTY_DEFAULT,
				NULL, fork_argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, &term->pid, NULL);
		
		g_free(fork_argv[0]);
		g_free(fork_argv[1]);
	}
}
