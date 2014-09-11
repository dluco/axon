#include <string.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "dialog.h"
#include "utils.h"

extern GSList *terminals;

void destroy(Terminal *term)
{
	/* Remove terminal from list */
	terminals = g_slist_remove(terminals, term);
	
	/* Check for empty list */
	if (terminals == NULL) {
		/* Last window - save config */
		config_save(term->conf, term->window);

		/* Destroy options and config */
		options_free(term->opts);
		config_free(term->conf);

		gtk_main_quit();
	} else {
		/* List not empty, so just destroy window */
		gtk_widget_destroy(term->window);
	}
}

gboolean delete_event(GtkWidget *widget, GdkEvent *event, Terminal *term)
{
	gint response;
	pid_t pgid;
	
	pgid = tcgetpgrp(vte_terminal_get_pty(VTE_TERMINAL(term->vte)));

	/* If running processes are found, ask before proceeding */
	if (pgid != -1 && pgid != term->pid) {
		response = dialog_message_question(term->window,
				"There are still processes running in this terminal. Do you really want to close?");

		if (response == GTK_RESPONSE_YES) {
			return FALSE;
		} else {
			return TRUE;
		}
	}

	return FALSE;	
}

void destroy_window(Terminal *term)
{
	char *output_file;
	GFile *file;
	GOutputStream *stream;
	GError *gerror = NULL;

	/* Check if terminal has an output file attached */
	output_file = g_object_get_data(G_OBJECT(term->vte), "output_file");

	if (output_file) {
		file = g_file_new_for_commandline_arg(output_file);
		/* Open a new output stream for overwriting the file - NO backup is made */
		stream = G_OUTPUT_STREAM(g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &gerror));

		if (stream) {
			vte_terminal_write_contents(VTE_TERMINAL(term->vte), stream, VTE_TERMINAL_WRITE_DEFAULT, NULL, &gerror);
			
			g_object_unref(stream);
		}

		/* Catch errors from multiple functions - shouldn't have overwriting of errors */
		if (gerror) {
			fprintf(stderr, "%s\n", gerror->message);
			g_error_free(gerror);
		}

		g_object_unref(file);
	}

	destroy(term);
}

void child_exited(GtkWidget *terminal, Terminal *term)
{
	int status;

	if (term->opts->hold) {
		/* Hold option activated */
		return;
	}

	waitpid(term->pid, &status, WNOHANG);
	/* TODO: check wait return */

	destroy_window(term);
}

/* Required when using multiple terminals */
void eof(GtkWidget *terminal, Terminal *term)
{
	int status;

	if (term->opts->hold) {
		/* Hold option activated */
		return;
	}
	
	waitpid(term->pid, &status, WNOHANG);
	/* TODO: check wait return */

	destroy_window(term);
}

void set_title(GtkWidget *terminal, GtkWidget *window)
{
	gtk_window_set_title(GTK_WINDOW(window), vte_terminal_get_window_title(VTE_TERMINAL(terminal)));
}

/* The size of a character in the terminal has changed - reset geometry hints */
void char_size_changed(GtkWidget *terminal, guint width, guint height, gpointer data)
{
	GtkWindow *window;
	GdkGeometry geometry;
	GtkBorder *inner_border;

	g_assert(GTK_IS_WINDOW(data));
	g_assert(VTE_IS_TERMINAL(terminal));

	window = GTK_WINDOW(data);
	if (!gtk_widget_get_realized(GTK_WIDGET (window))) {
		return;
	}

	gtk_widget_style_get(terminal, "inner-border", &inner_border, NULL);
	/* Resize increments - equal to one character in either direction */
	geometry.width_inc = width;
	geometry.height_inc = height;
	geometry.base_width = inner_border ? (inner_border->left + inner_border->right) : 0;
	geometry.base_height = inner_border ? (inner_border->top + inner_border->bottom) : 0;
	geometry.min_width = geometry.base_width + width * 2;
	geometry.min_height = geometry.base_height + height * 2;
	gtk_border_free(inner_border);

	gtk_window_set_geometry_hints(window, terminal, &geometry,
			GDK_HINT_RESIZE_INC |
			GDK_HINT_BASE_SIZE |
			GDK_HINT_MIN_SIZE);
}

void char_size_realized(GtkWidget *terminal, gpointer data)
{
	VteTerminal *vte;
	GtkWindow *window;
	GdkGeometry geometry;
	guint width, height;
	GtkBorder *inner_border;

	g_assert(GTK_IS_WINDOW(data));
	g_assert(VTE_IS_TERMINAL(terminal));

	vte = VTE_TERMINAL(terminal);
	window = GTK_WINDOW(data);
	if (!gtk_widget_get_realized(GTK_WIDGET(window))) {
		return;
	}

	gtk_widget_style_get(terminal, "inner-border", &inner_border, NULL);
	width = vte_terminal_get_char_width(vte);
	height = vte_terminal_get_char_height(vte);
	/* Resize increments - equal to one character in either direction */
	geometry.width_inc = width;
	geometry.height_inc = height;
	geometry.base_width = inner_border ? (inner_border->left + inner_border->right) : 0;
	geometry.base_height = inner_border ? (inner_border->top + inner_border->bottom) : 0;
	geometry.min_width = geometry.base_width + width * 2;
	geometry.min_height = geometry.base_height + height * 2;
	gtk_border_free(inner_border);

	gtk_window_set_geometry_hints(window, terminal, &geometry,
			GDK_HINT_RESIZE_INC |
			GDK_HINT_BASE_SIZE |
			GDK_HINT_MIN_SIZE);
}

void adjust_font_size(GtkWidget *terminal, GtkWidget *window, gint howmuch)
{
	VteTerminal *vte;
	PangoFontDescription *desired;
	gint newsize;
	gint old_columns, old_rows, owidth, oheight;

	/* Read the screen dimensions in cells. */
	vte = VTE_TERMINAL(terminal);
	old_columns = vte->column_count;
	old_rows = vte->row_count;

	/* Take into account padding and border overhead. */
	gtk_window_get_size(GTK_WINDOW(window), &owidth, &oheight);
	owidth -= vte->char_width * vte->column_count;
	oheight -= vte->char_height * vte->row_count;

	/* Calculate the new font size. */
	desired = pango_font_description_copy(vte_terminal_get_font(vte));
	newsize = pango_font_description_get_size(desired) / PANGO_SCALE;
	newsize += howmuch;
	pango_font_description_set_size(desired,
			CLAMP(newsize, 4, 144) * PANGO_SCALE);

	/* Change the font, then resize the window so that we have the same
	 * number of rows and columns. */
	vte_terminal_set_font(vte, desired);
	gtk_window_resize(GTK_WINDOW(window),
			old_columns * vte->char_width + owidth,
			old_rows * vte->char_height + oheight);

	pango_font_description_free(desired);
}

void increase_font_size(GtkWidget *terminal, GtkWidget *window)
{
	adjust_font_size(terminal, window, 1);
}

void decrease_font_size(GtkWidget *terminal, GtkWidget *window)
{
	adjust_font_size(terminal, window, -1);
}

void selection_changed(GtkWidget *terminal, GtkWidget *widget)
{
	gtk_widget_set_sensitive(widget, vte_terminal_get_has_selection(VTE_TERMINAL(terminal)));
}

gboolean button_press(GtkWidget *widget, GdkEventButton *event, Terminal *term)
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
		match = vte_terminal_match_check(VTE_TERMINAL(term->vte), column, row, &tag);

		if (match != NULL) {
			if (term->regex_tags[0] == tag) {
				/* url match */
				open_url(widget, match);
				g_free(match);
				return TRUE;
			} else if (term->regex_tags[1] == tag) {
				/* email match */
				open_email(widget, match);
				g_free(match);
				return TRUE;
			}
			g_free(match);
		}
		break;
	case 2:
		break;
	case 3:
		/* Right button: show popup menu */
		gtk_menu_popup(GTK_MENU(term->menu), NULL, NULL, NULL, NULL, event->button, event->time);
		/* TODO: show "open link" and "copy link" options in menu if match? */
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

gboolean key_press(GtkWidget *widget, GdkEventKey *event, Terminal *term)
{
	if (event->type != GDK_KEY_PRESS) {
		return FALSE;
	}

	/* Check if Caps-Lock is enabled - change keyval
	   to work with upper/lowercase. */
	if (gdk_keymap_get_caps_lock_state(gdk_keymap_get_default())) {
		event->keyval = gdk_keyval_to_upper(event->keyval);
	}

	/* Open new window: Ctrl+Shift+N */
	if ((event->state & NEW_WINDOW_ACCEL) == NEW_WINDOW_ACCEL) {
		if (event->keyval == NEW_WINDOW_KEY) {
			new_window(term);
			return TRUE;
		}
	}

	/* Copy text: Ctrl+Shift+C */
	if ((event->state & COPY_ACCEL) == COPY_ACCEL) {
		if (event->keyval == COPY_KEY) {
			copy_text(term);
			return TRUE;
		}
	}

	/* Copy text: Ctrl+Shift+V */
	if ((event->state & PASTE_ACCEL) == PASTE_ACCEL) {
		if (event->keyval == PASTE_KEY) {
			paste_text(term);
			return TRUE;
		}
	}

	/* Close Window: Ctrl+Shift+Q */
	if ((event->state & CLOSE_WINDOW_ACCEL) == CLOSE_WINDOW_ACCEL) {
		if (event->keyval == CLOSE_WINDOW_KEY) {
			if (!delete_event(NULL, NULL, term)) {
				destroy_window(term);
			}
			return TRUE;
		}
	}

	/* Keybindings without modifiers */
	switch(event->keyval) {
	case GDK_KEY_F11:
		/* Fullscreen */
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(term->fullscreen_item),
				!term->fullscreen);
		return TRUE;
	case GDK_KEY_Menu:
		/* Menu popup */
		gtk_menu_popup(GTK_MENU(term->menu), NULL, NULL,
				NULL, NULL, event->keyval, event->time);
		return TRUE;
	}

	return FALSE;
}

void new_window(Terminal *term)
{
	char *cwd;

	Terminal *n_term = terminal_new();
	terminal_init(n_term);
	
	cwd = terminal_get_cwd(term);
	terminal_load_config(n_term, term->conf);
	terminal_load_options(n_term, term->opts);
	
	terminal_run(n_term, cwd);
	terminal_show(n_term);

	g_free(cwd);
}

void copy_text(Terminal *term)
{
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

void paste_text(Terminal *term)
{
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

void fullscreen(Terminal *term)
{
	if (term->fullscreen == FALSE) {
		term->fullscreen = TRUE;
		gtk_window_fullscreen(GTK_WINDOW(term->window));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(term->window));
		term->fullscreen = FALSE;
	}
}

void config_file_changed(Config *conf)
{
	conf->modified_externally = TRUE;
}

void palette_changed(gchar *palette_file)
{
	Terminal *term;

	remove_suffix(palette_file, "theme");

	g_slist_foreach(terminals, (GFunc)terminal_set_palette, palette_file);
	
	/* Get first terminal's config and update color_scheme*/
	term = g_slist_nth_data(terminals, 0);
	g_free(term->conf->palette);
	term->conf->palette = g_strdup(palette_file);
	config_set_value(term->conf, "color_scheme", palette_file);
}

void open_url(GtkWidget *widget, char *url)
{
	GError *gerror = NULL;
	GdkScreen *screen;

	screen = gtk_widget_get_screen(GTK_WIDGET(widget));
	if (!gtk_show_uri(screen, url, gtk_get_current_event_time(), &gerror)) {
		dialog_message(gtk_widget_get_toplevel(widget), GTK_MESSAGE_WARNING,
				"Failed to open URL \"%s\"", url);
		g_error_free(gerror);
	}
}

void open_email(GtkWidget *widget, char *address)
{
	GError *gerror = NULL;
	GdkScreen *screen;
	gchar *tmp;

	/* ensure that a "mailto:" prefix is present */
	if (strncmp(address, "mailto:", 7) == 0) {
		tmp = g_strdup(address);
	} else {
		tmp = g_strconcat("mailto:", address, NULL);
	}

	screen = gtk_widget_get_screen(GTK_WIDGET(widget));
	if (!gtk_show_uri(screen, tmp, gtk_get_current_event_time(), &gerror)) {
		dialog_message(gtk_widget_get_toplevel(widget), GTK_MESSAGE_WARNING,
				"Failed to open email address \"%s\"", address);
		g_error_free(gerror);
	}
	g_free(tmp);
}
