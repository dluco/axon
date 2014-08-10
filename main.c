#include <gtk/gtk.h>
#include <vte/vte.h>

#include "callback.h"
#include "config.h"

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *scrolled_window;
	GtkWidget *terminal;
	char *command[] = {"/bin/bash", NULL,};

	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* initialize ui elements */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new(FALSE, 0);
	terminal = vte_terminal_new();
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
//	terminal = VTE_TERMINAL(terminal);

	/* setup */
	gtk_window_set_default_icon_name("terminal");

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	
	vte_terminal_set_size(VTE_TERMINAL(terminal), 80, 24);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(terminal), scrollback_lines);
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(terminal), scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(terminal), scroll_on_keystroke);
	vte_terminal_fork_command_full(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT, NULL, command, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL);

	/* arranging */
	gtk_container_add(GTK_CONTAINER(scrolled_window), terminal);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	
	/* signal setup */
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(quit), NULL);
	g_signal_connect(G_OBJECT(terminal), "child-exited", G_CALLBACK(quit), NULL);
	g_signal_connect(G_OBJECT(terminal), "window-title-changed", G_CALLBACK(set_title), window);
	g_signal_connect(G_OBJECT(terminal), "resize-window", G_CALLBACK(resize_window), window);

	/* make elements visible */
	gtk_widget_show(terminal);
	gtk_widget_show(scrolled_window);
	gtk_widget_show(vbox);
	gtk_widget_show(window);

	/* run gtk main loop */
	gtk_main();

	return 0;
}
