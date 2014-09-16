#include <string.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "dialog.h"
#include "utils.h"

extern GSList *terminals;

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

void new_window(Terminal *term)
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

void destroy_window(Terminal *term)
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

void copy_text(Terminal *term)
{
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

void paste_text(Terminal *term)
{
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

void open_url(GtkWidget *widget, char *url)
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

	screen = gtk_widget_get_screen(GTK_WIDGET(widget));
	if (!gtk_show_uri(screen, cmd, gtk_get_current_event_time(), &gerror)) {
		dialog_message(gtk_widget_get_toplevel(widget), GTK_MESSAGE_WARNING,
				"Failed to open URL \"%s\"", url);
		g_error_free(gerror);
	}
	g_free(cmd);
}
