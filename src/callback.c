#include <gtk/gtk.h>
#include <vte/vte.h>

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
