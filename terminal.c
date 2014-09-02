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

	/* FIXME: there must be a better way to handle resizing... */
	/* *************************************************** */
	/* Connect to the "char-size" changed signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(term->vte), 0, 0, term->window);
	g_signal_connect(G_OBJECT(term->vte), "char-size-changed",
			G_CALLBACK(char_size_changed), term->window);
	g_signal_connect(G_OBJECT(term->vte), "realize",
			G_CALLBACK(char_size_realized), term->window);
	/* *************************************************** */

	g_signal_connect(G_OBJECT(term->vte), "button-press-event",
			G_CALLBACK(button_press), term);
	g_signal_connect(G_OBJECT(term->vte), "window-title-changed",
			G_CALLBACK(set_title), term->window);
	g_signal_connect(G_OBJECT(term->vte), "child-exited",
			G_CALLBACK(child_exited), term);
	g_signal_connect(G_OBJECT(term->vte), "refresh-window",
			G_CALLBACK(refresh_window), term->window);
	g_signal_connect(G_OBJECT(term->vte), "resize-window",
			G_CALLBACK(resize_window), term->window);

//	gtk_widget_realize(term->vte);
	gtk_window_set_default_size(GTK_WINDOW(term->window),
			vte_terminal_get_column_count(VTE_TERMINAL(term->vte)),
			vte_terminal_get_row_count(VTE_TERMINAL(term->vte)));

	/* make elements visible */
	gtk_widget_show(term->vte);
	gtk_widget_show(term->scrollbar);
	gtk_widget_show(term->hbox);
	gtk_widget_show(term->window);
}

void terminal_load_config(Terminal *term, Config *conf)
{
	GRegex *regex;
	GError *gerror = NULL;
	int id;

	term->conf = conf;

	vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), conf->font);
	
//	terminal_load_colour_scheme(term, conf->colour_scheme);

	/* Scroll-related stuff */
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(term->vte), conf->scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(term->vte), conf->scroll_on_keystroke);
	(conf->show_scrollbar) ? gtk_widget_show(term->scrollbar) : gtk_widget_hide(term->scrollbar);
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
//	vte_terminal_set_size(VTE_TERMINAL(terminal), 80, 24);

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
	term->opts = opts;

	if (opts->title) {
		/* TODO: allow for title "modes" - append, replace (default), ignore */
		gtk_window_set_title(GTK_WINDOW(term->window), opts->title);
	}
	
	if (opts->font) {
//		term->conf->font = pango_font_description_from_string(opts->font);
		term->conf->font = g_strdup(opts->font);
	}

	/* mutually exclusive options, obviously */
	if (opts->fullscreen) {
		fullscreen(NULL, term);
	} else if (opts->maximize) {
		gtk_window_maximize(GTK_WINDOW(term->window));
	} else if (opts->geometry) {
		if (!gtk_window_parse_geometry(GTK_WINDOW(term->window), opts->geometry)) {
			print_err("invalid geometry format\n");
		} else {
			term->conf->columns = vte_terminal_get_column_count(VTE_TERMINAL(term->vte));
			term->conf->rows = vte_terminal_get_row_count(VTE_TERMINAL(term->vte));
		}
	}
}

void terminal_load_colour_scheme(Terminal *term, const char *colour_scheme)
{
	GKeyFile *cfg;
	GError *gerror = NULL;
	gchar *tmp = NULL;
	char *scheme_dir = NULL; /* 
							  * TODO: provide scheme_dir as a compile-time constant,
							  * defaulting to /usr/share/axon/colourschemes
							  */

	/* Config file initialization */
	cfg = g_key_file_new();

	scheme_dir = g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	if (!g_file_test(g_get_user_config_dir(), G_FILE_TEST_EXISTS)) {
		/* ~/.config does not exist - create it */
		g_mkdir(g_get_user_config_dir(), 0755);
	}
	if (!g_file_test(config_dir, G_FILE_TEST_EXISTS)) {
		/* Program config dir does not exist - create it */
		g_mkdir(config_dir, 0755);
	}
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
		conf->config_file = g_build_filename(scheme_dir, DEFAULT_CONFIG_FILE, NULL);
	}
		
	g_free(scheme_dir);

	/* Open config file */
	if (!g_key_file_load_from_file(cfg, conf->config_file, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
		/* If file does not exist, then ignore - one will be created */
		if (gerror->code == G_KEY_FILE_ERROR_UNKNOWN_ENCODING ||
			gerror->code == G_KEY_FILE_ERROR_INVALID_VALUE) {
			die("invalid config file format\n");
		}
		g_error_free(gerror);
		gerror = NULL;
	}

	if (!g_key_file_has_key(cfg, CFG_GROUP, "font", NULL)) {
		config_set_value(conf, "font", DEFAULT_FONT);
	}
	tmp = g_key_file_get_value(cfg, CFG_GROUP, "font", NULL);
	conf->font = g_strdup(tmp);
	free(tmp);

	if (!g_key_file_has_key(cfg, CFG_GROUP, "colour_scheme", NULL)) {
		config_set_value(conf, "colour_scheme", DEFAULT_COLOUR_SCHEME);
	}
	tmp = g_key_file_get_value(cfg, CFG_GROUP, "colour_scheme", NULL);
	conf->colour_scheme = g_strdup(tmp);
	free(tmp);
	
	if (!g_key_file_has_key(cfg, CFG_GROUP, "scroll_on_output", NULL)) {
		config_set_boolean(conf, "scroll_on_output", SCROLL_ON_OUTPUT);
	}
	conf->scroll_on_output = g_key_file_get_boolean(cfg, CFG_GROUP, "scroll_on_output", NULL);
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
