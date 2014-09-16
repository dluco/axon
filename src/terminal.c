#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <locale.h>
#include <langinfo.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>

#include "terminal.h"
#include "callback.h"
#include "menu.h"
#include "utils.h"

extern GSList *terminals;

static gboolean terminal_button_press_event(VteTerminal *vte, GdkEventButton *event, Terminal *term);
static gboolean terminal_key_press_event(GtkWidget *window, GdkEventKey *event, Terminal *term);
static void terminal_copy_text(Terminal *term);
static void terminal_paste_text(Terminal *term);
static void terminal_open_url(Terminal *term, char *url);
static void terminal_child_exited_event(GtkWidget *vte, Terminal *term);
static void terminal_eof_event(GtkWidget *vte, Terminal *term);
static void terminal_window_title_changed_event(GtkWidget *vte, Terminal *term);
static void terminal_selection_changed_event(GtkWidget *vte, GtkWidget *widget);
static void terminal_new_window(Terminal *term);
static void terminal_destroy_window(Terminal *term);
static void terminal_vte_initialize(Terminal *term);
static void terminal_menu_popup_initialize(Terminal *term);
static void terminal_settings_apply(Terminal *term);

static gboolean terminal_button_press_event(VteTerminal *vte, GdkEventButton *event, Terminal *term)
{
	glong column, row;
	gchar *match;
	gint tag;

	if (event->type != GDK_BUTTON_PRESS) {
		return FALSE;
	}
	
	switch(event->button) {
	case 1:
		/* find out if the cursor was over a matched expression */
		column = ((glong) (event->x) / vte_terminal_get_char_width(VTE_TERMINAL(term->vte)));
		row = ((glong) (event->y) / vte_terminal_get_char_height(VTE_TERMINAL(term->vte)));
		match = vte_terminal_match_check(vte, column, row, &tag);

		if (match != NULL) {
			terminal_open_url(term, match);
			g_free(match);
			return TRUE;
		}
		break;
	case 2:
		break;
	case 3:
		/* Right button: show popup menu */
		gtk_menu_popup(GTK_MENU(term->menu), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

static gboolean terminal_key_press_event(GtkWidget *window, GdkEventKey *event, Terminal *term)
{
	if (event->type != GDK_KEY_PRESS) {
		return FALSE;
	}

	/* Check if Caps-Lock is enabled - change keyval
	   to work with upper/lowercase. */
	if (gdk_keymap_get_caps_lock_state(gdk_keymap_get_default())) {
		event->keyval = gdk_keyval_to_upper(event->keyval);
	}

	/* Open new window: Ctrl+Shift+n */
	if ((event->state & NEW_WINDOW_ACCEL) == NEW_WINDOW_ACCEL) {
		if (event->keyval == NEW_WINDOW_KEY) {
			terminal_new_window(term);
			return TRUE;
		}
	}

	/* Copy text: Ctrl+Shift+c */
	if ((event->state & COPY_ACCEL) == COPY_ACCEL) {
		if (event->keyval == COPY_KEY) {
			terminal_copy_text(term);
			return TRUE;
		}
	}

	/* Paste text: Ctrl+Shift+v */
	if ((event->state & PASTE_ACCEL) == PASTE_ACCEL) {
		if (event->keyval == PASTE_KEY) {
			terminal_paste_text(term);
			return TRUE;
		}
	}

	/* Reset Terminal: Ctrl+Shift+r */
	if ((event->state & RESET_ACCEL) == RESET_ACCEL) {
		if (event->keyval == RESET_KEY) {
			vte_terminal_reset(VTE_TERMINAL(term->vte), TRUE, TRUE);
			return TRUE;
		}
	}

	/* Close Window: Ctrl+Shift+q */
	if ((event->state & CLOSE_WINDOW_ACCEL) == CLOSE_WINDOW_ACCEL) {
		if (event->keyval == CLOSE_WINDOW_KEY) {
			terminal_destroy_window(term);
			return TRUE;
		}
	}

	/* Keybindings without modifiers */
	switch(event->keyval) {
	case GDK_KEY_Menu:
		/* Menu popup */
		gtk_menu_popup(GTK_MENU(term->menu), NULL, NULL,
				NULL, NULL, event->keyval, event->time);
		return TRUE;
	}

	return FALSE;
}

static void terminal_copy_text(Terminal *term)
{
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

static void terminal_paste_text(Terminal *term)
{
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

static void terminal_open_url(Terminal *term, char *url)
{
	gchar *cmd;
	GError *gerror = NULL;
	GdkScreen *screen;

	if (strncmp(url, "http://", 7) != 0) {
		/* Prepend http to url or gtk_show_uri complains */
		cmd = g_strdup_printf("http://%s", url);
	} else {
		cmd = g_strdup(url);
	}

	screen = gtk_widget_get_screen(GTK_WIDGET(term->window));
	if (!gtk_show_uri(screen, cmd, gtk_get_current_event_time(), &gerror)) {
		print_err("Failed to open URL \"%s\"", url);
		g_error_free(gerror);
	}
	g_free(cmd);
}

static void terminal_child_exited_event(GtkWidget *vte, Terminal *term)
{
	int status;

	/* Only write config to file if last window */
	if (g_slist_length(terminals) == 1) {
		config_save(term->conf);
	}

	waitpid(term->pid, &status, WNOHANG);
	/* TODO: check wait return */

	terminal_destroy_window(term);

	/* Destroy if no windows left */
	if (g_slist_length(terminals) == 0) {
		gtk_main_quit();
	}
}

static void terminal_eof_event(GtkWidget *vte, Terminal *term)
{
	int status;

	/* Only write config to file if last window */
	if (g_slist_length(terminals) == 1) {
		config_save(term->conf);
	}
	
	waitpid(term->pid, &status, WNOHANG);
	/* TODO: check wait return */

	terminal_destroy_window(term);

	/* Destroy if no windows left */
	if (g_slist_length(terminals) == 0) {
		gtk_main_quit();
	}
}

static void terminal_window_title_changed_event(GtkWidget *vte, Terminal *term)
{
	if (term->conf->title_mode == TITLE_MODE_REPLACE) {
		gtk_window_set_title(GTK_WINDOW(term->window), vte_terminal_get_window_title(VTE_TERMINAL(vte)));
	}
}

static void terminal_selection_changed_event(GtkWidget *vte, GtkWidget *widget)
{
	/* Widget is sensitive if text is selected */
	gtk_widget_set_sensitive(widget, vte_terminal_get_has_selection(VTE_TERMINAL(vte)));
}

static void terminal_new_window(Terminal *term)
{
	Options n_opts;
	char *geometry;

	/* Initialize n_opts */
	memset(&n_opts, 0, sizeof(n_opts));

	n_opts.work_dir = terminal_get_cwd(term);
	Terminal *n_term = terminal_initialize(term->conf, &n_opts);
	
	/* Apply geometry */
	//gtk_widget_realize(n_term->vte);
	geometry = g_strdup_printf("%dx%d", DEFAULT_COLUMNS - 1, DEFAULT_ROWS);
	if (!gtk_window_parse_geometry(GTK_WINDOW(n_term->window), geometry)) {
		print_err("failed to set terminal size\n");
	}
	g_free(geometry);

	g_free(n_opts.work_dir);
}

static void terminal_destroy_window(Terminal *term)
{
	/* Only write config to file if last window */
	if (g_slist_length(terminals) == 1) {
		config_save(term->conf);
	}

	/* Remove terminal from list, destroy, and free */
	terminals = g_slist_remove(terminals, term);
	gtk_widget_destroy(term->window);

	/* Destroy if no windows left */
	if (g_slist_length(terminals) == 0) {
		gtk_main_quit();
	}
}

Terminal *terminal_initialize(Config *conf, Options *opts)
{
	Terminal *term;
	GdkColormap *colormap;
	char *geometry;

	/* Allocate and initialize new Terminal struct */
	term = g_new0(Terminal, 1);
	terminals = g_slist_append(terminals, term);
	term->conf = conf;
	/* TODO: remove dependency on opts */
	//term->opts = opts;

	/* Create toplevel window */
	term->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* Get an RGBA visual (colormap) and assign it to the new window */
	colormap = gdk_screen_get_rgba_colormap(gtk_widget_get_screen(GTK_WIDGET(term->window)));
	if (colormap != NULL) {
		gtk_widget_set_colormap(term->window, colormap);
	}

	/* Set window title */
	gtk_window_set_title(GTK_WINDOW(term->window), (opts->title) ? opts->title : "Axon");
	
	/* Set window icon */
	gtk_window_set_icon_name(GTK_WINDOW(term->window), "xterm");

	/* Create horizontal box as the child of the toplevel window */
	term->hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(term->window), term->hbox);

	/* Create VTE terminal as a child of the horizontal box */
	terminal_vte_initialize(term);
	gtk_box_pack_start(GTK_BOX(term->hbox), term->vte, TRUE, TRUE, 0);

	/* Create the scrollbar as a child of the horizontal box */
	term->scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(term->vte)));
	gtk_box_pack_start(GTK_BOX(term->hbox), term->scrollbar, FALSE, FALSE, 0);

	/* Create popup menu */
	terminal_menu_popup_initialize(term);
	
	/* Connect signals */
	g_signal_connect_swapped(G_OBJECT(term->window), "destroy", G_CALLBACK(terminal_destroy_window), term);
	g_signal_connect(G_OBJECT(term->window), "key-press-event", G_CALLBACK(terminal_key_press_event), term);

	if (opts->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(term->window));
	}

	/* FIXME: Apply geometry */
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

	/* Start terminal */
	terminal_run(term, opts);

	/* Show and realize all widgets */
	gtk_widget_show_all(term->window);

	/* Update terminal settings */
	terminal_settings_apply(term);

	return term;
}

static void terminal_vte_initialize(Terminal *term)
{
	GRegex *regex;
	gint id;

	/* Create VTE */
	term->vte = vte_terminal_new();

	/* Set up VTE */
	setlocale(LC_ALL, "");
	vte_terminal_set_emulation(VTE_TERMINAL(term->vte), "xterm");
	vte_terminal_set_encoding(VTE_TERMINAL(term->vte), nl_langinfo(CODESET));
    vte_terminal_set_backspace_binding(VTE_TERMINAL(term->vte), VTE_ERASE_ASCII_DELETE);
    vte_terminal_set_delete_binding(VTE_TERMINAL(term->vte), VTE_ERASE_DELETE_SEQUENCE);

	/* Match urls */
	regex = g_regex_new(URL_REGEX, G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY, NULL);
	id = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), regex, 0);
	vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), id, GDK_HAND1);
	g_regex_unref(regex);


	/* Connect to the "char-size-changed" signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(term->vte), 0, 0, term->window);
	g_signal_connect(G_OBJECT(term->vte), "char-size-changed", G_CALLBACK(char_size_changed), term->window);
	g_signal_connect(G_OBJECT(term->vte), "realize", G_CALLBACK(char_size_realized), term->window);

	/* Connect signals */
	g_signal_connect(G_OBJECT(term->vte), "button-press-event", G_CALLBACK(terminal_button_press_event), term);
	g_signal_connect(G_OBJECT(term->vte), "child-exited", G_CALLBACK(terminal_child_exited_event), term);
	g_signal_connect(G_OBJECT(term->vte), "eof", G_CALLBACK(terminal_eof_event), term);
	g_signal_connect(G_OBJECT(term->vte), "window-title-changed", G_CALLBACK(terminal_window_title_changed_event), term);
}

/* Initialize the popup menu */
static void terminal_menu_popup_initialize(Terminal *term)
{
	GtkWidget *new_window_item,
			*copy_item, *paste_item;
	GtkWidget *separator;
	GtkWidget *new_window_image;

	term->menu = gtk_menu_new();

	new_window_item = gtk_image_menu_item_new_with_mnemonic("Open _Terminal");
	copy_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	paste_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);

	/* Icon for new window item */
	new_window_image = gtk_image_new_from_icon_name("window-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(new_window_item), new_window_image);

	/* Add items to menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(term->menu), new_window_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(term->menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(term->menu), copy_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(term->menu), paste_item);

	/* Connect signals */
	g_signal_connect_swapped(G_OBJECT(new_window_item), "activate", G_CALLBACK(terminal_new_window), term);
	g_signal_connect_swapped(G_OBJECT(copy_item), "activate", G_CALLBACK(terminal_copy_text), term);
	g_signal_connect_swapped(G_OBJECT(paste_item), "activate", G_CALLBACK(terminal_paste_text), term);

	/* copy_item sensitivity - only sensitive when text is selected */
	g_signal_connect(G_OBJECT(term->vte), "selection-changed", G_CALLBACK(terminal_selection_changed_event), copy_item);
	gtk_widget_set_sensitive(copy_item, FALSE);

	gtk_widget_show_all(term->menu);
}

/* Apply settings to a Terminal */
static void terminal_settings_apply(Terminal *term)
{
	Config *conf = term->conf;

	/* VTE properties */
	vte_terminal_reset(VTE_TERMINAL(term->vte), FALSE, FALSE);

	/* Font */
	vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), conf->font);
	vte_terminal_set_allow_bold(VTE_TERMINAL(term->vte), conf->allow_bold);

	/* Scrollling */
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(term->vte), conf->scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(term->vte), conf->scroll_on_keystroke);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), conf->scrollback_lines);

	/* Cursor and mouse */
	vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte), (conf->blinking_cursor) ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF);
	vte_terminal_set_cursor_shape(VTE_TERMINAL(term->vte), conf->cursor_type);
	vte_terminal_set_mouse_autohide(VTE_TERMINAL(term->vte), conf->autohide_mouse);
	vte_terminal_set_word_chars(VTE_TERMINAL(term->vte), conf->word_chars);

	/* Bells */
	vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), conf->audible_bell);
	vte_terminal_set_visible_bell(VTE_TERMINAL(term->vte), conf->visible_bell);

	/* Set VTE colors and opacity */
	terminal_set_palette(term, conf->palette);
	terminal_set_opacity(term, conf->opacity);

	/* Show/hide the scrollbar */
	(conf->show_scrollbar) ? gtk_widget_show(term->scrollbar) : gtk_widget_hide(term->scrollbar);
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

void terminal_set_opacity(Terminal *term, int opacity)
{
	unsigned short alpha = (int)((opacity / 100.0) * 65535);

	if (gtk_widget_is_composited(term->window)) {
		/* Compositing enabled */
		vte_terminal_set_background_transparent(VTE_TERMINAL(term->vte), FALSE);
		vte_terminal_set_opacity(VTE_TERMINAL(term->vte), alpha);
	} else {
		vte_terminal_set_background_transparent(VTE_TERMINAL(term->vte), opacity != 100);
		vte_terminal_set_background_saturation(VTE_TERMINAL(term->vte),
				1 - ((double) alpha / 65535));
	}
}

void terminal_run(Terminal *term, Options *opts)
{
	int cmd_argc = 0;
	char **cmd_argv;
	char *cmd_joined;
	char *fork_argv[3];
	GError *gerror = NULL;
	char *path;

	if (opts->execute || opts->xterm_execute) {
		if (opts->execute) {
			/* -x option */
			if (!g_shell_parse_argv(opts->execute, &cmd_argc, &cmd_argv, &gerror)) {
				die("%s\n", gerror->message);
			}
		} else {
			/* -e option - last in commandline, scoops up rest of arguments */
			if (opts->xterm_args) {
				cmd_joined = g_strjoinv(" ", opts->xterm_args);
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
						opts->work_dir, cmd_argv, NULL, G_SPAWN_SEARCH_PATH,
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

		/* Set up args in preparation for fork. Real argv vector starts at
		 * argv[1] because we're using G_SPAWN_FILE_AND_ARGV_ZERO to be
		 * able to launch login shells. */
		fork_argv[0] = g_strdup(g_getenv("SHELL"));
		if (opts->login) {
			fork_argv[1] = g_strdup_printf("-%s", g_getenv("SHELL"));
		} else {
			fork_argv[1] = g_strdup(g_getenv("SHELL"));
		}
		fork_argv[2] = NULL;

		vte_terminal_fork_command_full(VTE_TERMINAL(term->vte), VTE_PTY_DEFAULT,
				opts->work_dir, fork_argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, &term->pid, NULL);
		
		g_free(fork_argv[0]);
		g_free(fork_argv[1]);
	}
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
