#include <gtk/gtk.h>
#include <vte/vte.h>

#include "menu.h"
#include "callback.h"

void menu_popup_init(GtkWidget *menu, Terminal *term)
{
	GtkWidget *copy_item, *paste_item, *fullscreen_item, *quit_item;

	copy_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	paste_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	fullscreen_item = gtk_check_menu_item_new_with_mnemonic("_Fullscreen");
	quit_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);

	/* assign labels with mnemonics */
	gtk_menu_item_set_label(GTK_MENU_ITEM(copy_item), "_Copy");
	gtk_menu_item_set_label(GTK_MENU_ITEM(paste_item), "_Paste");
	gtk_menu_item_set_label(GTK_MENU_ITEM(quit_item), "_Quit");

	/* accels? */

	gtk_menu_append(GTK_MENU(menu), copy_item);
	gtk_menu_append(GTK_MENU(menu), paste_item);
	gtk_menu_append(GTK_MENU(menu), fullscreen_item);
	gtk_menu_append(GTK_MENU(menu), quit_item);

	/* set up signals */
	g_signal_connect(G_OBJECT(copy_item), "activate", G_CALLBACK(copy_text), term);
	g_signal_connect(G_OBJECT(paste_item), "activate", G_CALLBACK(paste_text), term);
	g_signal_connect(G_OBJECT(fullscreen_item), "activate", G_CALLBACK(fullscreen), term);
	g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(destroy_window), term);

	gtk_widget_show(copy_item);
	gtk_widget_show(paste_item);
	gtk_widget_show(fullscreen_item);
	gtk_widget_show(quit_item);
}
