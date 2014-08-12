#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "callback.h"
#include "config.h"
#include "terminal.h"
#include "utils.h"

Terminal *terminal_new(void)
{
	Terminal *terminal;
	char *command[] = {"/bin/bash", NULL};

	if (!(terminal = malloc(sizeof(*terminal)))) {
		die("failure to malloc terminal");
	}

	/* initialize ui elements */
	terminal->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	terminal->hbox = gtk_hbox_new(FALSE, 0);
	terminal->vte = vte_terminal_new();
	terminal->scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(terminal->vte)));

	/* setup */
	gtk_window_set_icon_name(GTK_WINDOW(terminal->window), "terminal");
	
//	vte_terminal_set_size(VTE_TERMINAL(terminal), 80, 24);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal->vte), scrollback_lines);
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal->vte), scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal->vte), scroll_on_keystroke);
	vte_terminal_fork_command_full(VTE_TERMINAL(terminal->vte),
			VTE_PTY_DEFAULT, NULL, command,
			NULL, G_SPAWN_DEFAULT, NULL,
			NULL, NULL, NULL);

	/* arranging */
	gtk_box_pack_start(GTK_BOX(terminal->hbox), terminal->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(terminal->hbox), terminal->scrollbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(terminal->window), terminal->hbox);
	
	/* signal setup */
	g_signal_connect(G_OBJECT(terminal->window), "delete-event",
			G_CALLBACK(delete_event), terminal->vte);

	/* Connect to the "char-size" changed signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(terminal->vte), 0, 0, terminal->window);
	g_signal_connect(G_OBJECT(terminal->vte), "char-size-changed",
			G_CALLBACK(char_size_changed), terminal->window);
	g_signal_connect(G_OBJECT(terminal->vte), "realize",
			G_CALLBACK(char_size_realized), terminal->window);

	g_signal_connect(G_OBJECT(terminal->vte), "child-exited",
			G_CALLBACK(child_exited), terminal->window);
	g_signal_connect(G_OBJECT(terminal->vte), "window-title-changed",
			G_CALLBACK(set_title), terminal->window);
	g_signal_connect(G_OBJECT(terminal->vte), "refresh-window",
			G_CALLBACK(refresh_window), terminal->window);
	g_signal_connect(G_OBJECT(terminal->vte), "resize-window",
			G_CALLBACK(resize_window), terminal->window);

//	gtk_widget_realize(terminal->vte);
	gtk_window_set_default_size(GTK_WINDOW(terminal->window),
			vte_terminal_get_column_count(VTE_TERMINAL(terminal->vte)),
			vte_terminal_get_row_count(VTE_TERMINAL(terminal->vte)));

	/* make elements visible */
	gtk_widget_show(terminal->vte);
	(show_scrollbar) ? gtk_widget_show(terminal->scrollbar) : gtk_widget_hide(terminal->scrollbar);
	gtk_widget_show(terminal->hbox);
	gtk_widget_show(terminal->window);
	
	return terminal;
}
