#ifndef CALLBACK_H
#define CALLBACK_H

void destroy_and_quit(VteTerminal *, GtkWidget *);
void delete_event(GtkWidget *, GdkEvent *, gpointer);
void child_exited(GtkWidget *, gpointer *);
void set_title(GtkWidget *, GtkWidget *);
void char_size_changed(GtkWidget *, guint, guint, gpointer);
void char_size_realized(GtkWidget *, gpointer);
void refresh_window(GtkWidget *, gpointer);
void resize_window(GtkWidget *, guint, guint, gpointer);

#endif /* CALLBACK_H */
