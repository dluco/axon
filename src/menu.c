#include <gtk/gtk.h>
#include <vte/vte.h>

#include "menu.h"
#include "callback.h"
#include "dialog.h"

void menu_popup_init(GtkWidget *menu, Terminal *term)
{
	GtkWidget *new_window_item,
			*copy_item, *paste_item,
			*fullscreen_item;
	GtkWidget *separator;
	GtkWidget *new_window_image;

	new_window_item = gtk_image_menu_item_new_with_mnemonic("Open _Terminal");
	copy_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	paste_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	fullscreen_item = gtk_check_menu_item_new_with_label("Fullscreen");

	/* Icon for new window item */
	new_window_image = gtk_image_new_from_icon_name("window-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(new_window_item), new_window_image);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), new_window_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), copy_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), paste_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), fullscreen_item);

	/* set up signals */
	g_signal_connect_swapped(G_OBJECT(new_window_item), "activate", G_CALLBACK(new_window), term);
	g_signal_connect_swapped(G_OBJECT(copy_item), "activate", G_CALLBACK(copy_text), term);
	g_signal_connect_swapped(G_OBJECT(paste_item), "activate", G_CALLBACK(paste_text), term);
	g_signal_connect_swapped(G_OBJECT(fullscreen_item), "activate", G_CALLBACK(fullscreen), term);

	/* copy_item sensitivity */
	g_signal_connect(G_OBJECT(term->vte), "selection-changed", G_CALLBACK(selection_changed), copy_item);
	gtk_widget_set_sensitive(copy_item, FALSE);

	/* Bookmark fullscreen_item for when -s option is specified */
	term->fullscreen_item = fullscreen_item;

	gtk_widget_show_all(menu);
}
