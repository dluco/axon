#include <gtk/gtk.h>
#include <vte/vte.h>

#include "menu.h"
#include "callback.h"
#include "dialog.h"

void menu_popup_init(GtkWidget *menu, Terminal *term)
{
	GtkWidget *new_window_item,
			*copy_item, *paste_item,
			*fullscreen_item, *preferences_item,
			*about_item, *quit_item;
	GtkWidget *separator;
	GtkWidget *new_window_image;

	new_window_item = gtk_image_menu_item_new_with_mnemonic("_Open Terminal");
	copy_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	paste_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	fullscreen_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_FULLSCREEN, NULL);
	preferences_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	about_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	quit_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);

	/* assign labels with mnemonics */
//	gtk_menu_item_set_label(GTK_MENU_ITEM(copy_item), "_Copy");
//	gtk_menu_item_set_label(GTK_MENU_ITEM(paste_item), "_Paste");
	gtk_menu_item_set_label(GTK_MENU_ITEM(preferences_item), "Pr_eferences...");
//	gtk_menu_item_set_label(GTK_MENU_ITEM(quit_item), "_Quit");

	/* Icon for new window item */
	new_window_image = gtk_image_new_from_icon_name("window-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(new_window_item), new_window_image);

	/* accels? - handled by toplevel window */

	gtk_menu_append(GTK_MENU(menu), new_window_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), separator);
	gtk_widget_show(separator);

	gtk_menu_append(GTK_MENU(menu), copy_item);
	gtk_menu_append(GTK_MENU(menu), paste_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), separator);
	gtk_widget_show(separator);

	gtk_menu_append(GTK_MENU(menu), fullscreen_item);
	gtk_menu_append(GTK_MENU(menu), preferences_item);
	gtk_menu_append(GTK_MENU(menu), about_item);
	separator = gtk_separator_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), separator);
	gtk_widget_show(separator);
	
	gtk_menu_append(GTK_MENU(menu), quit_item);

	/* set up signals */
	g_signal_connect(G_OBJECT(new_window_item), "activate", G_CALLBACK(new_window), term);
	g_signal_connect(G_OBJECT(copy_item), "activate", G_CALLBACK(copy_text), term);
	g_signal_connect(G_OBJECT(paste_item), "activate", G_CALLBACK(paste_text), term);
	g_signal_connect(G_OBJECT(fullscreen_item), "activate", G_CALLBACK(fullscreen), term);
	g_signal_connect(G_OBJECT(preferences_item), "activate", G_CALLBACK(preferences), term);
	g_signal_connect(G_OBJECT(about_item), "activate", G_CALLBACK(dialog_about), NULL);
	g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(destroy_window), term);

	/* copy_item sensitivity */
	g_signal_connect(G_OBJECT(term->vte), "selection-changed", G_CALLBACK(selection_changed), copy_item);
	gtk_widget_set_sensitive(copy_item, FALSE);

	gtk_widget_show(new_window_item);
	gtk_widget_show(copy_item);
	gtk_widget_show(paste_item);
	gtk_widget_show(fullscreen_item);
	gtk_widget_show(preferences_item);
	gtk_widget_show(about_item);
	gtk_widget_show(quit_item);
}
