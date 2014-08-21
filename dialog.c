#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

void dialog_preferences(Terminal *term)
{
	GtkWidget *vbox;
	GtkWidget *font_button;

	GtkWidget* dialog = gtk_dialog_new_with_buttons(PACKAGE" settings",
			GTK_WINDOW(term->window), GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	font_button = gtk_font_button_new_with_font(term->conf->font);

	gtk_box_pack_start(GTK_BOX(vbox), font_button, TRUE, TRUE, 0);
	gtk_widget_show(font_button);
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(dialog);
	 	
	return;
}

void dialog_preferences_font(Terminal *term)
{
	static GtkWidget *dialog = NULL;
	int response;

	if (dialog == NULL) {
		dialog = gtk_font_selection_dialog_new("Select Font");

		gtk_window_set_icon_name(GTK_WINDOW(dialog), "fonts");
		//gtk_window_set_icon_name(GTK_WINDOW(dialog), "preferences-desktop");

		gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), term->conf->font);
	}

	response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK) {
		g_free(term->conf->font);
		term->conf->font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), term->conf->font);
		config_set_value(term->conf, "font", term->conf->font);
	}

//	gtk_widget_destroy(dialog);
	gtk_widget_hide(dialog);
}
