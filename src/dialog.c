#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

/*
void dialog_preferences(Terminal *term)
{
	GtkWidget *vbox;
	GtkWidget *font_button;

	GtkWidget* dialog = gtk_dialog_new_with_buttons("axon settings",
			GTK_WINDOW(term->window), GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	
	gtk_window_set_icon_name(GTK_WINDOW(dialog), "preferences-desktop");
	
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	font_button = gtk_font_button_new_with_font(term->conf->font);

	gtk_box_pack_start(GTK_BOX(vbox), font_button, TRUE, TRUE, 0);
	gtk_widget_show(font_button);
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(dialog);
	 	
	return;
}
*/

void dialog_preferences_font(Terminal *term)
{
	GtkWidget *dialog = gtk_font_selection_dialog_new("Select Font");

	gtk_window_set_icon_name(GTK_WINDOW(dialog), "fonts");
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), term->conf->font);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		g_free(term->conf->font);
		term->conf->font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		vte_terminal_set_font_from_string(VTE_TERMINAL(term->vte), term->conf->font);
		config_set_value(term->conf, "font", term->conf->font);
	}

	gtk_widget_destroy(dialog);
}

void dialog_about(void)
{
	const char *authors[] = {"David Luco <dluco11@gmail.com>", NULL};
	char *website = "http://dluco.github.io/axon/";

	GtkWidget *dialog = gtk_about_dialog_new();
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	
	/* set name, version, and comments */
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "axon");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A simple terminal emulator");

	/* set logo to available icon */
	if (gtk_icon_theme_has_icon(theme, "xterm")) {
		gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "xterm");
		gtk_window_set_icon_name(GTK_WINDOW(dialog), "xterm");
	} else {
		gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "utilities-terminal");
		gtk_window_set_icon_name(GTK_WINDOW(dialog), "utilities-terminal");
	}
	
	/* authors, license, and website */
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog),
			"Distributed under the MIT license.\nhttp://www.opensource.org/licenses/mit-license.php");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), website);
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), "Axon website");

	gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(dialog);	
}
