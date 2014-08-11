#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "callback.h"
#include "config.h"

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *hbox;
	GtkWidget *scrollbar;
	GtkWidget *terminal;
	char *command[] = {"/bin/bash", NULL,};

	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* initialize ui elements */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	hbox = gtk_hbox_new(FALSE, 0);
	terminal = vte_terminal_new();
	scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(terminal)));
//	terminal = VTE_TERMINAL(terminal);

	/* setup */
	gtk_window_set_default_icon_name("terminal");
	
//	vte_terminal_set_size(VTE_TERMINAL(terminal), 80, 24);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), scrollback_lines);
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), scroll_on_keystroke);
	vte_terminal_fork_command_full(VTE_TERMINAL(terminal),
			VTE_PTY_DEFAULT, NULL, command,
			NULL, G_SPAWN_DEFAULT, NULL,
			NULL, NULL, NULL);

	/* arranging */
	gtk_box_pack_start(GTK_BOX(hbox), terminal, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), scrollbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	
	/* signal setup */
	g_signal_connect(G_OBJECT(window), "delete-event",
			G_CALLBACK(delete_event), terminal);

	/* Connect to the "char-size" changed signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(terminal), 0, 0, window);
	g_signal_connect(G_OBJECT(terminal), "char-size-changed",
			G_CALLBACK(char_size_changed), window);
	g_signal_connect(G_OBJECT(terminal), "realize",
			G_CALLBACK(char_size_realized), window);

	g_signal_connect(G_OBJECT(terminal), "child-exited",
			G_CALLBACK(child_exited), window);
	g_signal_connect(G_OBJECT(terminal), "window-title-changed",
			G_CALLBACK(set_title), window);
	g_signal_connect(G_OBJECT(terminal), "refresh-window",
			G_CALLBACK(refresh_window), window);
	g_signal_connect(G_OBJECT(terminal), "resize-window",
			G_CALLBACK(resize_window), window);

//	gtk_widget_realize(terminal);
	gtk_window_set_default_size(GTK_WINDOW(window),
			vte_terminal_get_column_count(VTE_TERMINAL(terminal)),
			vte_terminal_get_row_count(VTE_TERMINAL(terminal)));

	/* make elements visible */
	gtk_widget_show(terminal);
	(show_scrollbar) ? gtk_widget_show(scrollbar) : gtk_widget_hide(scrollbar);
	gtk_widget_show(hbox);
	gtk_widget_show(window);

	/* run gtk main loop */
	gtk_main();

	return 0;
}
