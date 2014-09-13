#include <string.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "dialog.h"
#include "utils.h"

static char* search_string = NULL;
static GRegex *regex = NULL;
static gboolean case_sensitive = FALSE;
static gboolean wrap_search = TRUE;

static void state_toggled(GtkWidget *widget, gboolean *data)
{
	*data = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void search_entry_changed(GtkWidget *entry, GtkWidget *dialog)
{
	const char *text;
	gboolean has_text;

	text = gtk_entry_get_text(GTK_ENTRY(entry));
	has_text = (strcmp(text, "") != 0) ? TRUE : FALSE;

	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, has_text);
}

void search_find_next(Terminal *term)
{
	vte_terminal_search_find_next(VTE_TERMINAL(term->vte));
}

void search_dialog(Terminal *term)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *search_label;
	GtkWidget *search_entry;
	GtkWidget *case_sensitive_button;
	GtkWidget *wrap_search_button;
	gint result;
	GError *gerror = NULL;
	
	dialog = gtk_dialog_new_with_buttons("Find",
			GTK_WINDOW(term->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_FIND, GTK_RESPONSE_OK,
			NULL);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);

	search_label = gtk_label_new_with_mnemonic("Fi_nd what:");
	search_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), search_entry, FALSE, FALSE, 5);

	gtk_label_set_mnemonic_widget(GTK_LABEL(search_label), search_entry);
	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, FALSE);

	g_signal_connect(G_OBJECT(search_entry), "changed",
			G_CALLBACK(search_entry_changed), dialog);

	if (search_string) {
		gtk_entry_set_text(GTK_ENTRY(search_entry), search_string);
		gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, TRUE);
	}

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_entry_set_activates_default(GTK_ENTRY(search_entry), TRUE);
	
	case_sensitive_button = gtk_check_button_new_with_mnemonic("_Case sensitive");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(case_sensitive_button), case_sensitive);
	g_signal_connect(G_OBJECT(case_sensitive_button), "toggled",
			G_CALLBACK(state_toggled), &case_sensitive);
	gtk_container_set_border_width(GTK_CONTAINER(case_sensitive_button), 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), case_sensitive_button, FALSE, FALSE, 5);

	wrap_search_button = gtk_check_button_new_with_mnemonic("_Wrap search");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wrap_search_button), wrap_search);
	g_signal_connect(G_OBJECT(wrap_search_button), "toggled",
			G_CALLBACK(state_toggled), &wrap_search);
	gtk_container_set_border_width(GTK_CONTAINER(wrap_search_button), 4);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), wrap_search_button, FALSE, FALSE, 5);
	
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(hbox);
	gtk_widget_show(case_sensitive_button);
	gtk_widget_show(wrap_search_button);
	
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	
	if (result == GTK_RESPONSE_OK) {
		g_free(search_string);
		search_string = g_strdup(gtk_entry_get_text(GTK_ENTRY(search_entry)));

		vte_terminal_search_set_wrap_around(VTE_TERMINAL(term->vte), wrap_search);
		if (regex != NULL) {
			g_regex_unref(regex);
			regex = NULL;
		}
		regex = g_regex_new(search_string, (case_sensitive) ? 0 : G_REGEX_CASELESS,
				G_REGEX_MATCH_NOTEMPTY, &gerror);
		if (gerror) {
			print_err("%s\n", gerror->message);
			g_error_free(gerror);
			return;
		}
		vte_terminal_search_set_gregex(VTE_TERMINAL(term->vte), regex);

		if (!vte_terminal_search_find_next(VTE_TERMINAL(term->vte))) {
			dialog_message(term->window, GTK_MESSAGE_WARNING, "Search string not found");
		}
	}
	
	gtk_widget_destroy(dialog);
}
