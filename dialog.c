#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

void dialog_preferences_font(Terminal *term)
{
	GtkWidget *dialog;
	int response;

	dialog = gtk_font_selection_dialog_new("Select Font");

//	gtk_window_set_icon_name(GTK_WINDOW(dialog), "fonts");
	gtk_window_set_icon_name(GTK_WINDOW(dialog), "preferences-desktop");

	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), term->conf->font);

	response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK) {
		g_free(term->conf->font);
		term->conf->font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), term->conf->font);
		config_set_value(term->conf, "font", term->conf->font);
	}

	gtk_widget_destroy(dialog);
}
