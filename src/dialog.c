#include <stdarg.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

extern GSList *terminals;

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
