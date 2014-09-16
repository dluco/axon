#ifndef CALLBACK_H
#define CALLBACK_H

void new_window(Terminal *);
void destroy_window(Terminal *);
void char_size_changed(GtkWidget *, guint, guint, gpointer);
void char_size_realized(GtkWidget *, gpointer);
void adjust_font_size(GtkWidget *, GtkWidget *, gint);
void increase_font_size(GtkWidget *, GtkWidget *);
void decrease_font_size(GtkWidget *, GtkWidget *);
void copy_text(Terminal *);
void paste_text(Terminal *);
void open_url(GtkWidget *, char *);

#endif /* CALLBACK_H */
