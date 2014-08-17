#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

void dialog_preferences(Terminal *term)
{
	GtkWidget *dialog;
	int result;

	dialog = gtk_font_selection_dialog_new("Select Font");

//	gtk_window_set_icon_name(GTK_WINDOW(dialog), "fonts");
	gtk_window_set_icon_name(GTK_WINDOW(dialog), "preferences-desktop");

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
	case GTK_RESPONSE_OK:
		g_print("accept\n");
		break;
	default:
		break;
	}

	gtk_widget_destroy(dialog);
}
