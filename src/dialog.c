#include <stdarg.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

extern GSList *terminals;

void dialog_font(Terminal *term)
{
	GtkWidget *dialog = gtk_font_selection_dialog_new("Select Font");

	gtk_window_set_icon_name(GTK_WINDOW(dialog), "fonts");
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(dialog), term->conf->font);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		g_free(term->conf->font);
		term->conf->font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
		g_slist_foreach(terminals, (GFunc)terminal_set_font, term->conf->font);
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
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Axon");
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

/* GTK_MESSAGE_INFO, GTK_MESSAGE_WARNING, GTK_MESSAGE_ERROR */
void dialog_message(GtkWidget *window, GtkMessageType type, gchar *message, ...)
{
	va_list ap;
	GtkWidget *dialog;
	gchar *str;
	
	va_start(ap, message);
		str = g_strdup_vprintf(message, ap);
	va_end(ap);
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		type,
		GTK_BUTTONS_NONE,
		str);

	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_OK,
			GTK_RESPONSE_CANCEL, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

	g_free(str);
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

gint dialog_message_question(GtkWidget *window, gchar *message, ...)
{
	va_list ap;
	GtkWidget *dialog;
	gchar *str;
	gint result;
	
	va_start(ap, message);
		str = g_strdup_vprintf(message, ap);
	va_end(ap);
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			str);

	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
	
	g_free(str);
	
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	
	return result;
}
