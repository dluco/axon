#include <gtk/gtk.h>
#include <vte/vte.h>

void quit()
{
	gtk_main_quit();
}

void set_title(GtkWidget *terminal, GtkWidget *window)
{
	gtk_window_set_title(GTK_WINDOW(window), vte_terminal_get_window_title(VTE_TERMINAL(terminal)));
}

void resize_window(GtkWidget *widget, guint width, guint height, gpointer data)
{
	VteTerminal *terminal;

	g_print("here!\n");

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
