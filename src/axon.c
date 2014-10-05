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

#include "axon.h"

GSList *terminals;

static void die(const char *errstr, ...);
static void print_err(const char *errstr, ...);
static gboolean terminal_button_press_event(VteTerminal *vte, GdkEventButton *event, Terminal *term);
static gboolean terminal_key_press_event(GtkWidget *window, GdkEventKey *event, Terminal *term);
static void terminal_copy_text(Terminal *term);
static void terminal_paste_text(Terminal *term);
static void terminal_fullscreen(Terminal *term);
static void terminal_open_url(Terminal *term, char *url);
static void terminal_child_exited_event(GtkWidget *vte, Terminal *term);
static void terminal_eof_event(GtkWidget *vte, Terminal *term);
static void terminal_window_title_changed_event(GtkWidget *vte, Terminal *term);
static void terminal_selection_changed_event(GtkWidget *vte, GtkWidget *widget);
static void terminal_new_window(Terminal *term);
static void terminal_destroy_window(Terminal *term);
static Terminal *terminal_initialize(Config *conf, Options *opts);
static void terminal_vte_initialize(Terminal *term);
static void terminal_menu_popup_initialize(Terminal *term);
static void terminal_settings_apply(Terminal *term);
static void terminal_set_palette(Terminal *term, char *palette_name);
static void terminal_set_opacity(Terminal *term, int opacity);
static void terminal_run(Terminal *term, Options *opts);
static char *terminal_get_cwd(Terminal *term);
static void config_initialize(Config *conf);
static Config *config_load_from_file(const char *user_file);
static Options *options_parse(int argc, char *argv[]);

static void die(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");
	
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void print_err(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
}

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
		if (term->conf->highlight_urls) {
			/* find out if the cursor was over a matched expression */
			column = ((glong) (event->x) / vte_terminal_get_char_width(VTE_TERMINAL(term->vte)));
			row = ((glong) (event->y) / vte_terminal_get_char_height(VTE_TERMINAL(term->vte)));
			match = vte_terminal_match_check(vte, column, row, &tag);

			if (match != NULL) {
				terminal_open_url(term, match);
				g_free(match);
				return TRUE;
			}
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
	case FULLSCREEN_KEY:
		/* Toggle fullscreen */
		terminal_fullscreen(term);
		return TRUE;
	case MENU_KEY:
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

static void terminal_fullscreen(Terminal *term)
{
	if (term->fullscreen == FALSE) {
		term->fullscreen = TRUE;
		gtk_window_fullscreen(GTK_WINDOW(term->window));
	} else {
		term->fullscreen = FALSE;
		gtk_window_unfullscreen(GTK_WINDOW(term->window));
	}
}

static void terminal_open_url(Terminal *term, char *url)
{
	gchar *cmd;

	if (strncmp(url, "http", 4) == 0 || strncmp(url, "ftp", 3) == 0) {
		/* url has a proper http or ftp prefix */
		cmd = g_strdup(url);
	} else {
		/* prepend http to url or gtk_show_uri complains */
		cmd = g_strdup_printf("http://%s", url);
	}

	if (!gtk_show_uri(gtk_widget_get_screen(GTK_WIDGET(term->window)), cmd, gtk_get_current_event_time(), NULL)) {
		print_err("unable to open URL \"%s\"", url);
	}

	g_free(cmd);
}

static void terminal_child_exited_event(GtkWidget *vte, Terminal *term)
{
	int status;

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
	g_free(n_opts.work_dir);
	
	/* Apply geometry */
	//gtk_widget_realize(n_term->vte);
	geometry = g_strdup_printf("%dx%d", DEFAULT_COLUMNS - 1, DEFAULT_ROWS);
	if (!gtk_window_parse_geometry(GTK_WINDOW(n_term->window), geometry)) {
		print_err("failed to set terminal size\n");
	}
	g_free(geometry);
}

static void terminal_destroy_window(Terminal *term)
{
	/* Remove terminal from list, destroy, and free */
	terminals = g_slist_remove(terminals, term);
	gtk_widget_destroy(term->window);

	/* Destroy if no windows left */
	if (g_slist_length(terminals) == 0) {
		gtk_main_quit();
	}
}

static Terminal *terminal_initialize(Config *conf, Options *opts)
{
	Terminal *term;
	GdkColormap *colormap;
	GdkGeometry hints;
	GtkBorder *border;
	char *geometry;

	/* Allocate and initialize new Terminal struct */
	term = g_new0(Terminal, 1);
	terminals = g_slist_append(terminals, term);
	term->conf = conf;

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
	
	/* Apply fullscreen option, if set */
	if (opts->fullscreen) {
		gtk_window_fullscreen(GTK_WINDOW(term->window));
	}

	/* Apply terminal settings - font is set before hints,
	   otherwise the window won't resize correctly. */
	terminal_settings_apply(term);

	/* Start terminal */
	terminal_run(term, opts);

	/* Show and realize hbox */
	gtk_widget_show_all(term->hbox);

	/* Set geometry hints */
	gtk_widget_style_get(term->vte, "inner-border", &border, NULL);
	hints.width_inc = vte_terminal_get_char_width(VTE_TERMINAL(term->vte));  
	hints.height_inc = vte_terminal_get_char_height(VTE_TERMINAL(term->vte));
	hints.base_width = border->right + 1;
	hints.base_height = border->bottom + 1;
	hints.min_width = hints.base_width + hints.width_inc * 4;
	hints.min_height = hints.base_height + hints.height_inc * 2;
	gtk_border_free(border);

	gtk_window_set_geometry_hints(GTK_WINDOW(term->window),
		term->vte,
		&hints,
		GDK_HINT_RESIZE_INC | GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);

	/* Apply geometry option, if set, otherwise set default size */
	if (opts->geometry) {
		geometry = g_strdup(opts->geometry);
		g_free(opts->geometry);

		if (!gtk_window_parse_geometry(GTK_WINDOW(term->window), geometry)) {
			print_err("invalid geometry format\n");
		}
		g_free(geometry);
	} else {
		/* Set default window size */
		vte_terminal_set_size(VTE_TERMINAL(term->vte), DEFAULT_COLUMNS - 1, DEFAULT_ROWS);
	}

	/* Connect signals */
	g_signal_connect_swapped(G_OBJECT(term->window), "destroy", G_CALLBACK(terminal_destroy_window), term);
	g_signal_connect(G_OBJECT(term->window), "key-press-event", G_CALLBACK(terminal_key_press_event), term);
	g_signal_connect_swapped(G_OBJECT(term->window), "composited-changed", G_CALLBACK(terminal_settings_apply), term);

	gtk_widget_show_all(term->window);

	/* Show/hide the scrollbar */
	(conf->show_scrollbar) ? gtk_widget_show(term->scrollbar) : gtk_widget_hide(term->scrollbar);

	return term;
}

static void terminal_vte_initialize(Terminal *term)
{
	/* Create VTE */
	term->vte = vte_terminal_new();

	/* Set up VTE */
	setlocale(LC_ALL, "");
	vte_terminal_set_emulation(VTE_TERMINAL(term->vte), "xterm");
	vte_terminal_set_encoding(VTE_TERMINAL(term->vte), nl_langinfo(CODESET));
    vte_terminal_set_backspace_binding(VTE_TERMINAL(term->vte), VTE_ERASE_ASCII_DELETE);
    vte_terminal_set_delete_binding(VTE_TERMINAL(term->vte), VTE_ERASE_DELETE_SEQUENCE);

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
	GRegex *regex;
	gint id;

	Config *conf = term->conf;

	/* VTE properties */
	vte_terminal_reset(VTE_TERMINAL(term->vte), FALSE, FALSE);

	/* Font */
	vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), conf->font);
	vte_terminal_set_allow_bold(VTE_TERMINAL(term->vte), conf->allow_bold);

	/* Scrolling */
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

	/* Highlight urls */
	if (conf->highlight_urls) {
		regex = g_regex_new(URL_REGEX, G_REGEX_CASELESS, G_REGEX_MATCH_NOTEMPTY, NULL);
		id = vte_terminal_match_add_gregex(VTE_TERMINAL(term->vte), regex, 0);
		vte_terminal_match_set_cursor_type(VTE_TERMINAL(term->vte), id, GDK_HAND1);
		g_regex_unref(regex);
	}

	/* Set VTE colors and opacity */
	terminal_set_palette(term, conf->palette);
	terminal_set_opacity(term, conf->opacity);
}

static void terminal_set_palette(Terminal *term, char *palette_name)
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

static void terminal_set_opacity(Terminal *term, int opacity)
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

static void terminal_run(Terminal *term, Options *opts)
{
	int cmd_argc = 0;
	char **cmd_argv;
	char *cmd_joined;
	char *fork_argv[3];
	GError *gerror = NULL;
	char *path;

	if (opts->command || opts->execute) {
		/* Check for --execute first: when both --execute and --command
		   are given, execute takes precedence */
		if (opts->execute) {
			/* -e option - last in command line, collects remainder of arguments */
			if (opts->execute_args) {
				cmd_joined = g_strjoinv(" ", opts->execute_args);
				if (!g_shell_parse_argv(cmd_joined, &cmd_argc, &cmd_argv, &gerror)) {
					die("%s\n", gerror->message);
				}
				g_free(cmd_joined);
			}
		} else {
			/* -x option */
			if (!g_shell_parse_argv(opts->command, &cmd_argc, &cmd_argv, &gerror)) {
				die("%s\n", gerror->message);
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
		   argv[1] because we're using G_SPAWN_FILE_AND_ARGV_ZERO to be
		   able to launch login shells. */
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
static char *terminal_get_cwd(Terminal *term)
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

/* Set default values for Config */
static void config_initialize(Config *conf)
{
	conf->font = DEFAULT_FONT;
	conf->palette = DEFAULT_COLOR_SCHEME;
	conf->opacity = DEFAULT_OPACITY;
	conf->title_mode = DEFAULT_TITLE_MODE;
	conf->scroll_on_output = SCROLL_ON_OUTPUT;
	conf->scroll_on_keystroke = SCROLL_ON_KEYSTROKE;
	conf->show_scrollbar = SCROLLBAR;
	conf->scrollback_lines = SCROLLBACK_LINES;
	conf->allow_bold = ALLOW_BOLD;
	conf->highlight_urls = HIGHLIGHT_URLS;;
	conf->audible_bell = AUDIBLE_BELL;
	conf->visible_bell = VISIBLE_BELL;
	conf->blinking_cursor = BLINKING_CURSOR;
	conf->cursor_type = DEFAULT_CURSOR_TYPE;
	conf->autohide_mouse = AUTOHIDE_MOUSE;
	conf->word_chars = WORD_CHARS;
}

static Config *config_load_from_file(const char *user_file)
{
	Config *conf;
	GKeyFile *keyfile;
	gchar *tmp = NULL;
	char *config_file = NULL;
	GError *gerror = NULL;

	/* Allocate Config */
	conf = g_new0(Config, 1);

	/* Initialize Config with default values, in case we can't load a file */
	config_initialize(conf);

	if (user_file) {
		/* Build path to user config file */
		if (g_path_is_absolute(user_file)) {
			/* Absolute path was given */
			config_file = g_strdup(user_file);
		} else {
			/* Relative path to file was given - prepend current directory */
			tmp = g_get_current_dir();
			config_file = g_build_filename(tmp, user_file, NULL);
			g_free(tmp);
		}
		/* Test if user supplied config file actually exists and is not a directory */
		if (!g_file_test(config_file, G_FILE_TEST_IS_REGULAR)) {
			print_err("invalid config file \"%s\"\n", user_file);
		}
	} else {
		/* Try $XDG_CONFIG_HOME/axon/axonrc first */
		config_file = g_build_filename(g_get_user_config_dir(), "axon", DEFAULT_CONFIG_FILE, NULL);

		/* Check if file exists */
		if (!g_file_test(config_file, G_FILE_TEST_IS_REGULAR)) {
			/* Try $HOME/.axonrc */
			g_free(config_file);
			config_file = g_build_filename(g_get_home_dir(), "." DEFAULT_CONFIG_FILE, NULL);

			/* Check if file exists */
			if (!g_file_test(config_file, G_FILE_TEST_IS_REGULAR)) {
				/* No config file available - defaults will be used */
				g_free(config_file);
				return conf;
			}
		}
	}

	/* Initialize keyfile */
	keyfile = g_key_file_new();

	/* Open config file */
	if (!g_key_file_load_from_file(keyfile, config_file, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
		/* Couldn't open config file - defaults will be used */
		print_err("error opening config file\n");
		g_error_free(gerror);
		g_free(config_file);
		return conf;
	}

	g_free(config_file);

	/* Load key values */
	if (g_key_file_has_key(keyfile, CFG_GROUP, "font", NULL)) {
		conf->font = g_key_file_get_value(keyfile, CFG_GROUP, "font", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "color_scheme", NULL)) {
		conf->palette = g_key_file_get_value(keyfile, CFG_GROUP, "color_scheme", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "opacity", NULL)) {
		conf->opacity = g_key_file_get_integer(keyfile, CFG_GROUP, "opacity", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "title_mode", NULL)) {
		tmp = g_key_file_get_value(keyfile, CFG_GROUP, "title_mode", NULL);
		if (strcmp(tmp, "replace") == 0) {
			conf->title_mode = TITLE_MODE_REPLACE;
		} else {
			conf->title_mode = TITLE_MODE_IGNORE;
		}
		free(tmp);
	}
	
	if (g_key_file_has_key(keyfile, CFG_GROUP, "scroll_on_output", NULL)) {
		conf->scroll_on_output = g_key_file_get_boolean(keyfile, CFG_GROUP, "scroll_on_output", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "scroll_on_keystroke", NULL)) {
		conf->scroll_on_keystroke = g_key_file_get_boolean(keyfile, CFG_GROUP, "scroll_on_keystroke", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "scrollbar", NULL)) {
		conf->show_scrollbar = g_key_file_get_boolean(keyfile, CFG_GROUP, "scrollbar", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "scrollback_lines", NULL)) {
		conf->scrollback_lines = g_key_file_get_integer(keyfile, CFG_GROUP, "scrollback_lines", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "allow_bold", NULL)) {
		conf->allow_bold = g_key_file_get_boolean(keyfile, CFG_GROUP, "allow_bold", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "highlight_urls", NULL)) {
		conf->highlight_urls = g_key_file_get_boolean(keyfile, CFG_GROUP, "highlight_urls", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "audible_bell", NULL)) {
		conf->audible_bell = g_key_file_get_boolean(keyfile, CFG_GROUP, "audible_bell", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "visible_bell", NULL)) {
		conf->visible_bell = g_key_file_get_boolean(keyfile, CFG_GROUP, "visible_bell", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "blinking_cursor", NULL)) {
		conf->blinking_cursor = g_key_file_get_boolean(keyfile, CFG_GROUP, "blinking_cursor", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "cursor_type", NULL)) {
		tmp = g_key_file_get_value(keyfile, CFG_GROUP, "cursor_type", NULL);
		if (strcmp(tmp, "block") == 0) {
			conf->cursor_type = VTE_CURSOR_SHAPE_BLOCK;
		} else if (strcmp(tmp , "underline") == 0) {
			conf->cursor_type = VTE_CURSOR_SHAPE_UNDERLINE;
		} else if (strcmp(tmp, "beam") == 0) {
			conf->cursor_type = VTE_CURSOR_SHAPE_IBEAM;
		}
		free(tmp);
	}
	
	if (g_key_file_has_key(keyfile, CFG_GROUP, "autohide_mouse", NULL)) {
		conf->autohide_mouse = g_key_file_get_boolean(keyfile, CFG_GROUP, "autohide_mouse", NULL);
	}

	if (g_key_file_has_key(keyfile, CFG_GROUP, "word_chars", NULL)) {
		conf->word_chars = g_key_file_get_value(keyfile, CFG_GROUP, "word_chars", NULL);
	}

	g_key_file_free(keyfile);

	return conf;
}

static Options *options_parse(int argc, char *argv[])
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
		printf("axon %s, 2014 David Luco <dluco11@gmail.com>\n", VERSION);
		exit(EXIT_SUCCESS);
	}

	/* A working directory must always be provided */
	if (!opts->work_dir) {
		opts->work_dir = g_get_current_dir();
	}

	return opts;
}

/* Main entry point for program */
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

	return 0;
}
