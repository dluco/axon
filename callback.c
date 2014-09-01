#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "dialog.h"
#include "utils.h"

#define WRITE_OUT FALSE

void destroy(Terminal *term)
{
	const char *output_file;
	GFile *file;
	GOutputStream *stream;
	GError *gerror = NULL;

	output_file = "out.txt";
	
	if (output_file && WRITE_OUT) {
		file = g_file_new_for_commandline_arg (output_file);
		stream = G_OUTPUT_STREAM (g_file_replace (file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &gerror));

		if (stream) {
			vte_terminal_write_contents (VTE_TERMINAL(term->vte), stream,
						     VTE_TERMINAL_WRITE_DEFAULT,
						     NULL, &gerror);
			g_object_unref (stream);
		}

		if (gerror) {
			fprintf(stderr, "%s\n", gerror->message);
			g_error_free (gerror);
		}

		g_object_unref (file);
	}

//	gtk_widget_destroy(term->window);
	/* TODO: remove for multiple windows */
	gtk_main_quit();
}

gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	return FALSE;	
}

void destroy_window(GtkWidget *widget, Terminal *term)
{
	destroy(term);
}

void child_exited(GtkWidget *terminal, Terminal *term)
{
	/* TODO: perform a waitpid on term's child */
	destroy(term);
}

void set_title(GtkWidget *terminal, GtkWidget *window)
{
	gtk_window_set_title(GTK_WINDOW(window), vte_terminal_get_window_title(VTE_TERMINAL(terminal)));
}

void char_size_changed(GtkWidget *widget, guint width, guint height, gpointer data)
{
	GtkWindow *window;
	GdkGeometry geometry;
	GtkBorder *inner_border;

	g_assert(GTK_IS_WINDOW(data));
	g_assert(VTE_IS_TERMINAL(widget));

	window = GTK_WINDOW(data);
	if (!gtk_widget_get_realized (GTK_WIDGET (window)))
		return;

	gtk_widget_style_get (widget, "inner-border", &inner_border, NULL);
	geometry.width_inc = width;
	geometry.height_inc = height;
	geometry.base_width = inner_border ? (inner_border->left + inner_border->right) : 0;
	geometry.base_height = inner_border ? (inner_border->top + inner_border->bottom) : 0;
	geometry.min_width = geometry.base_width + width * 2;
	geometry.min_height = geometry.base_height + height * 2;
	gtk_border_free (inner_border);

	gtk_window_set_geometry_hints(window, widget, &geometry,
				      GDK_HINT_RESIZE_INC |
				      GDK_HINT_BASE_SIZE |
				      GDK_HINT_MIN_SIZE);
}

void char_size_realized(GtkWidget *widget, gpointer data)
{
	VteTerminal *terminal;
	GtkWindow *window;
	GdkGeometry geometry;
	guint width, height;
	GtkBorder *inner_border;

	g_assert(GTK_IS_WINDOW(data));
	g_assert(VTE_IS_TERMINAL(widget));

	terminal = VTE_TERMINAL(widget);
	window = GTK_WINDOW(data);
	if (!gtk_widget_get_realized (GTK_WIDGET(window)))
		return;

	gtk_widget_style_get (widget, "inner-border", &inner_border, NULL);
	width = vte_terminal_get_char_width (terminal);
	height = vte_terminal_get_char_height (terminal);
	geometry.width_inc = width;
	geometry.height_inc = height;
	geometry.base_width = inner_border ? (inner_border->left + inner_border->right) : 0;
	geometry.base_height = inner_border ? (inner_border->top + inner_border->bottom) : 0;
	geometry.min_width = geometry.base_width + width * 2;
	geometry.min_height = geometry.base_height + height * 2;
	gtk_border_free (inner_border);

	gtk_window_set_geometry_hints(window, widget, &geometry,
				      GDK_HINT_RESIZE_INC |
				      GDK_HINT_BASE_SIZE |
				      GDK_HINT_MIN_SIZE);
}

void refresh_window(GtkWidget *widget, gpointer data)
{
	GdkWindow *window;
	GtkAllocation allocation;
	GdkRectangle rect;

	if (GTK_IS_WIDGET(data)) {
		window = gtk_widget_get_window(widget);
		if (window) {
			gtk_widget_get_allocation(widget, &allocation);
			rect.x = rect.y = 0;
			rect.width = allocation.width;
			rect.height = allocation.height;
			gdk_window_invalidate_rect(window, &rect, TRUE);
		}
	}
}

void resize_window(GtkWidget *widget, guint width, guint height, gpointer data)
{
	VteTerminal *terminal;

	if ((GTK_IS_WINDOW(data)) && (width >= 2) && (height >= 2)) {
		gint owidth, oheight, char_width, char_height, column_count, row_count;
		GtkBorder *inner_border;

		terminal = VTE_TERMINAL(widget);

		gtk_window_get_size(GTK_WINDOW(data), &owidth, &oheight);

		/* Take into account border overhead. */
		char_width = vte_terminal_get_char_width (terminal);
		char_height = vte_terminal_get_char_height (terminal);
		column_count = vte_terminal_get_column_count (terminal);
		row_count = vte_terminal_get_row_count (terminal);
		gtk_widget_style_get (widget, "inner-border", &inner_border, NULL);

		owidth -= char_width * column_count;
		oheight -= char_height * row_count;
		if (inner_border != NULL) {
			owidth -= inner_border->left + inner_border->right;
			oheight -= inner_border->top + inner_border->bottom;
		}
		gtk_window_resize(GTK_WINDOW(data), width + owidth, height + oheight);
		gtk_border_free (inner_border);
	}
}

void selection_changed(GtkWidget *terminal, GtkWidget *widget)
{
	gtk_widget_set_sensitive(widget, vte_terminal_get_has_selection(VTE_TERMINAL(terminal)));
}

gboolean button_press(GtkWidget *widget, GdkEventButton *event, Terminal *term)
{
	glong column, row;
	gint tag;

	if (event->type != GDK_BUTTON_PRESS) {
		return FALSE;
	}
	
	/* find out if the cursor was over a matched expression */
	column = ((glong) (event->x) / vte_terminal_get_char_width(VTE_TERMINAL(term->vte)));
	row = ((glong) (event->y) / vte_terminal_get_char_height(VTE_TERMINAL(term->vte)));
	term->match = vte_terminal_match_check(VTE_TERMINAL(term->vte), column, row, &tag);

	switch(event->button) {
	case 1:
		/* open url if any */
		if (term->match != NULL) {
			open_url(NULL, term->match);
			return TRUE;
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
	/* FIXME: free(match)? */

	return FALSE;
}

gboolean key_press(GtkWidget *widget, GdkEventKey *event, Terminal *term)
{
	if (event->type != GDK_KEY_PRESS) {
		return FALSE;
	}

	switch(event->keyval) {
	case GDK_KEY_Menu:
		gtk_menu_popup(GTK_MENU(term->menu), NULL, NULL, NULL, NULL, event->keyval, event->time);
		return TRUE;
	}

	return FALSE;
}

void copy_text(GtkWidget *widget, Terminal *term)
{
	vte_terminal_copy_clipboard(VTE_TERMINAL(term->vte));
}

void paste_text(GtkWidget *widget, Terminal *term)
{
	vte_terminal_paste_clipboard(VTE_TERMINAL(term->vte));
}

void fullscreen(GtkWidget *widget, Terminal *term)
{
	if (term->fullscreen == FALSE) {
		term->fullscreen = TRUE;
		gtk_window_fullscreen(GTK_WINDOW(term->window));
	} else {
		gtk_window_unfullscreen(GTK_WINDOW(term->window));
		term->fullscreen = FALSE;
	}
}

void preferences(GtkWidget *widget, Terminal *term)
{
	dialog_preferences_font(term);
//	dialog_preferences(term);
}

void open_url(GtkWidget *widget, char *match)
{
	GError *gerror = NULL;
	gchar *cmd;
	gchar *browser = NULL;

	browser = (gchar *)g_getenv("BROWSER");

	if (browser) {
		cmd = g_strdup_printf("%s %s", browser, match);
	} else {
		if ( (browser = g_find_program_in_path("xdg-open")) ) {
			cmd = g_strdup_printf("%s %s", browser, match);
			g_free(browser);
		} else {
			cmd = g_strdup_printf("firefox %s", match);
		}
	}

	if (!g_spawn_command_line_async(cmd, &gerror)) {
		print_err("Couldn't exec \"%s\": %s\n", cmd, gerror->message);
		g_error_free(gerror);
	}

	g_free(cmd);
}
