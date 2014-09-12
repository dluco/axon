#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include <glib/gstdio.h>

#include "terminal.h"
#include "callback.h"
#include "menu.h"
#include "utils.h"

extern GSList *terminals;

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

	/* set state variables etc */
	term->fullscreen = FALSE;

	/* setup */
	gtk_window_set_icon_name(GTK_WINDOW(term->window), "xterm");

	menu_popup_init(term->menu, term);
	
	/* arranging */
	gtk_box_pack_start(GTK_BOX(term->hbox), term->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(term->hbox), term->scrollbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(term->window), term->hbox);
	
	/* Signal setup */
	g_signal_connect(G_OBJECT(term->window), "delete-event",
			G_CALLBACK(delete_event), term);
	g_signal_connect_swapped(G_OBJECT(term->window), "destroy",
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
			G_CALLBACK(set_title), term);
	g_signal_connect(G_OBJECT(term->vte), "increase-font-size",
			G_CALLBACK(increase_font_size), term->window);
	g_signal_connect(G_OBJECT(term->vte), "decrease-font-size",
			G_CALLBACK(decrease_font_size), term->window);

	/* Add newly init-ed terminal to list */
	terminals = g_slist_append(terminals, term);
}

void terminal_load_config(Terminal *term, Config *conf)
{
	GRegex *regex;
	GError *gerror = NULL;

	term->conf = conf;

	vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), conf->font);
	terminal_set_palette(term, conf->palette);

	/* Scroll-related stuff */
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(term->vte), conf->scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(term->vte), conf->scroll_on_keystroke);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), conf->scrollback_lines);

	/* Set the (annoying) bells */
	vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), conf->audible_bell);
	vte_terminal_set_visible_bell(VTE_TERMINAL(term->vte), conf->visible_bell);
	
	/* Disable the (stupid) blinking cursor... */
	vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte),
			(conf->blinking_cursor) ?
				VTE_CURSOR_BLINK_ON :
				VTE_CURSOR_BLINK_OFF);
	vte_terminal_set_cursor_shape(VTE_TERMINAL(term->vte), conf->cursor_type);
	vte_terminal_set_mouse_autohide(VTE_TERMINAL(term->vte), conf->autohide_mouse);
	vte_terminal_set_word_chars(VTE_TERMINAL(term->vte), conf->word_chars);

	/* Build the url regex */
	regex = g_regex_new(URL_REGEX, G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY, &gerror);
	if (gerror) {
		print_err("%s\n", gerror->message);
		g_error_free(gerror);
		return;
	}
	term->regex_tags[0] = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), regex, 0);
	/* Release the regex owned by vte now */
	g_regex_unref(regex);
	vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), term->regex_tags[0], GDK_HAND1);

	/* Build the email regex */
	regex = g_regex_new(EMAIL_REGEX, G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY, &gerror);
	if (gerror) {
		print_err("%s\n", gerror->message);
		g_error_free(gerror);
		return;
	}
	term->regex_tags[1] = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), regex, 0);
	/* Release the regex owned by vte now */
	g_regex_unref(regex);
	vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), term->regex_tags[1], GDK_HAND1);
}

void terminal_load_options(Terminal *term, Options *opts)
{
	char *geometry;

	term->opts = opts;

	if (opts->title) {
		gtk_window_set_title(GTK_WINDOW(term->window), opts->title);
		g_free(opts->title);
	}
	
	if (opts->font) {
		/* Set font in conf, but do not update key_file. */
		term->conf->font = g_strdup(opts->font);
		vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), opts->font);
		g_free(opts->font);
	}

	/* Mutually exclusive options */
	if (opts->fullscreen) {
		/* Correctly set state of fullscreen menu item
		 * and trigger callback function */
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(term->fullscreen_item), TRUE);
	} else if (opts->maximize) {
		gtk_window_maximize(GTK_WINDOW(term->window));
	}

	/* Apply geometry */
	gtk_widget_realize(term->vte);
	if (opts->geometry) {
		geometry = g_strdup(opts->geometry);
		g_free(opts->geometry);
	} else {
		geometry = g_strdup_printf("%dx%d", DEFAULT_COLUMNS - 1, DEFAULT_ROWS);
	}
	if (!gtk_window_parse_geometry(GTK_WINDOW(term->window), geometry)) {
		print_err("invalid geometry format\n");
	}
	g_free(geometry);

	if (opts->output_file) {
		g_object_set_data(G_OBJECT(term->vte), "output_file", opts->output_file);
	}
}

void terminal_set_palette(Terminal *term, char *palette_name)
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
	gchar *palette_file;
	gchar **palette_colors;
	gchar *tmp;
	int n = 0;
	GError *gerror = NULL;

	/* Config file initialization */
	cfg = g_key_file_new();

	tmp = g_strdup_printf("%s.%s", palette_name, "theme");
	palette_file = g_build_filename(DATADIR, "axon", "colorschemes", tmp, NULL);
	g_free(tmp);

	/* Open config file */
	if (!g_key_file_load_from_file(cfg, palette_file, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
		/* palette file not found - default terminal colors will be used */
		print_err("palette \"%s\": %s\n", palette, gerror->message);
		g_error_free(gerror);
		gerror = NULL;
	} else {
		/* Begin parsing palette file */
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
			if (has_fg) {
				vte_terminal_set_color_foreground(VTE_TERMINAL(term->vte), &fg);
			}
			if (has_bg) {
				vte_terminal_set_color_background(VTE_TERMINAL(term->vte), &bg);
			}
		}

		/* Apply cursor color */
		vte_terminal_set_color_cursor(VTE_TERMINAL(term->vte), has_cursor ? &cursor : NULL);
	}

	g_key_file_free(cfg);
	g_free(palette_file);
}

void terminal_run(Terminal *term, char *cwd)
{
	int cmd_argc = 0;
	char **cmd_argv;
	char *cmd_joined;
	char *fork_argv[3];
	GError *gerror = NULL;
	char *path;

	if (!cwd) {
		cwd = g_get_current_dir();
	}

	if (term->opts->execute || term->opts->xterm_execute) {
		if (term->opts->execute) {
			/* -x option */
			if (!g_shell_parse_argv(term->opts->execute, &cmd_argc, &cmd_argv, &gerror)) {
				die("%s\n", gerror->message);
			}
			/* Reset for new windows */
			g_free(term->opts->execute);
			term->opts->execute = FALSE;
		} else {
			/* -e option - last in commandline, scoops up rest of arguments */
			if (term->opts->xterm_args) {
				cmd_joined = g_strjoinv(" ", term->opts->xterm_args);
				if (!g_shell_parse_argv(cmd_joined, &cmd_argc, &cmd_argv, &gerror)) {
					die("%s\n", gerror->message);
				}
				g_free(cmd_joined);
			}
			/* Reset for new windows */
			g_strfreev(term->opts->xterm_args);
			term->opts->xterm_execute = FALSE;
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
		}
	} else { 
		/* No execute option */
		if (term->opts->hold) {
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
				cwd, fork_argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, &term->pid, NULL);
		
		g_free(fork_argv[0]);
		g_free(fork_argv[1]);
	}
}

void terminal_set_font(Terminal *term, char *font)
{
	VteTerminal *vte;
	int old_columns, old_rows, owidth, oheight;

	vte = VTE_TERMINAL(term->vte);

	/* Save column and row count before setting font */
	old_columns = vte->column_count;
	old_rows = vte->row_count;

	/* Take into account padding and border overhead. */
	gtk_window_get_size(GTK_WINDOW(term->window), &owidth, &oheight);
	owidth -= vte->char_width * old_columns;
	oheight -= vte->char_height * old_rows;

	/* Apply font */
	vte_terminal_set_font_from_string(vte, font);
	
	/* Restore window geometry */
	gtk_window_resize(GTK_WINDOW(term->window),
			old_columns * vte->char_width + owidth,
			old_rows * vte->char_height + oheight);
}

void terminal_show(Terminal *term)
{
	gtk_widget_show_all(term->window);
	(term->conf->show_scrollbar) ?
			gtk_widget_show(term->scrollbar) : gtk_widget_hide(term->scrollbar);
}

/* Retrieve the cwd of the specified term page.
 * Original function was from terminal-screen.c of gnome-terminal, copyright (C) 2001 Havoc Pennington
 * Adapted by Hong Jen Yee, non-linux stuff removed by David Gómez */
char *terminal_get_cwd(Terminal *term)
{
	char *cwd = NULL;

	if (term->pid >= 0) {
		char *file, *buf;
		struct stat sb;
		int len;

		file = g_strdup_printf ("/proc/%d/cwd", term->pid);

		if (g_stat(file, &sb) == -1) {
			g_free(file);
			return cwd;
		}

		buf = malloc(sb.st_size + 1);

		if(buf == NULL) {
			g_free(file);
			return cwd;
		}

		len = readlink(file, buf, sb.st_size + 1);

		if (len > 0 && buf[0] == '/') {
			buf[len] = '\0';
			cwd = g_strdup(buf);
		}

		g_free(buf);
		g_free(file);
	}

	return cwd;
}
