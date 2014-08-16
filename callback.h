#ifndef CALLBACK_H
#define CALLBACK_H

void destroy(Terminal *);
gboolean delete_event(GtkWidget *, GdkEvent *, gpointer);
void destroy_window(GtkWidget *, Terminal *);
void child_exited(GtkWidget *, Terminal *);
void set_title(GtkWidget *, GtkWidget *);
void char_size_changed(GtkWidget *, guint, guint, gpointer);
void char_size_realized(GtkWidget *, gpointer);
void refresh_window(GtkWidget *, gpointer);
void resize_window(GtkWidget *, guint, guint, gpointer);
gboolean button_press(GtkWidget *, GdkEventButton *, Terminal *);
gboolean key_press(GtkWidget *, GdkEventKey *, Terminal *);
void copy_text(GtkWidget *, Terminal *);
void paste_text(GtkWidget *, Terminal *);
void fullscreen(GtkWidget *, Terminal *);
void open_url(GtkWidget *, char *);

#endif /* CALLBACK_H */
