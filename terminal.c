#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"
#include "callback.h"
#include "utils.h"

Terminal *terminal_new(void)
{
	Terminal *term;

	if (!(term = malloc(sizeof(*term)))) {
		die("failure to malloc terminal");
	}

	return term;
}

void terminal_init(Terminal *term)
{
	/* initialize ui elements */
	term->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	term->hbox = gtk_hbox_new(FALSE, 0);
	term->vte = vte_terminal_new();
	term->scrollbar = gtk_vscrollbar_new(vte_terminal_get_adjustment(VTE_TERMINAL(term->vte)));

	/* setup */
//	gtk_window_set_icon_name(GTK_WINDOW(term->window), "terminal");
	gtk_window_set_icon_name(GTK_WINDOW(term->window), "xterm");
	
	/* arranging */
	gtk_box_pack_start(GTK_BOX(term->hbox), term->vte, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(term->hbox), term->scrollbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(term->window), term->hbox);
	
	/* signal setup */
	g_signal_connect(G_OBJECT(term->window), "delete-event",
			G_CALLBACK(delete_event), NULL);
	g_signal_connect(G_OBJECT(term->window), "destroy",
			G_CALLBACK(destroy_window), term);

	/* *************************************************** */
	/* Connect to the "char-size" changed signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	char_size_changed(GTK_WIDGET(term->vte), 0, 0, term->window);
	g_signal_connect(G_OBJECT(term->vte), "char-size-changed",
			G_CALLBACK(char_size_changed), term->window);
	g_signal_connect(G_OBJECT(term->vte), "realize",
			G_CALLBACK(char_size_realized), term->window);
	/* *************************************************** */

	g_signal_connect(G_OBJECT(term->vte), "child-exited",
			G_CALLBACK(child_exited), term);
	g_signal_connect(G_OBJECT(term->vte), "window-title-changed",
			G_CALLBACK(set_title), term->window);
	g_signal_connect(G_OBJECT(term->vte), "refresh-window",
			G_CALLBACK(refresh_window), term->window);
	g_signal_connect(G_OBJECT(term->vte), "resize-window",
			G_CALLBACK(resize_window), term->window);

//	gtk_widget_realize(term->vte);
	gtk_window_set_default_size(GTK_WINDOW(term->window),
			vte_terminal_get_column_count(VTE_TERMINAL(term->vte)),
			vte_terminal_get_row_count(VTE_TERMINAL(term->vte)));

	/* make elements visible */
	gtk_widget_show(term->vte);
	gtk_widget_show(term->scrollbar);
	gtk_widget_show(term->hbox);
	gtk_widget_show(term->window);
}

void terminal_load_config(Terminal *term, Config *conf)
{
	vte_terminal_set_scroll_on_output(VTE_TERMINAL(term->vte), conf->scroll_on_output);
	vte_terminal_set_scroll_on_keystroke(VTE_TERMINAL(term->vte), conf->scroll_on_keystroke);
	(conf->show_scrollbar) ? gtk_widget_show(term->scrollbar) : gtk_widget_hide(term->scrollbar);
	vte_terminal_set_scrollback_lines(VTE_TERMINAL(term->vte), conf->scrollback_lines);
	vte_terminal_set_audible_bell(VTE_TERMINAL(term->vte), conf->audible_bell);
	vte_terminal_set_visible_bell(VTE_TERMINAL(term->vte), conf->visible_bell);
	vte_terminal_set_cursor_blink_mode(VTE_TERMINAL(term->vte),
			(conf->blinking_cursor) ?
				VTE_CURSOR_BLINK_ON :
				VTE_CURSOR_BLINK_OFF);
//	vte_terminal_set_size(VTE_TERMINAL(terminal), 80, 24);
}

void terminal_run(Terminal *term, char *cmd)
{
	char *command[] = {"/bin/bash", NULL};

	vte_terminal_fork_command_full(VTE_TERMINAL(term->vte),
			VTE_PTY_DEFAULT, NULL, command,
			NULL, G_SPAWN_DEFAULT, NULL,
			NULL, NULL, NULL);
}
