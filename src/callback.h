#ifndef CALLBACK_H
#define CALLBACK_H

void destroy(void);
gboolean delete_event(GtkWidget *, GdkEvent *, Terminal *);
void destroy_window(Terminal *);
void child_exited(GtkWidget *, Terminal *);
void eof(GtkWidget *, Terminal *);
void set_title(GtkWidget *, Terminal *);
void char_size_changed(GtkWidget *, guint, guint, gpointer);
void char_size_realized(GtkWidget *, gpointer);
void adjust_font_size(GtkWidget *, GtkWidget *, gint);
void increase_font_size(GtkWidget *, GtkWidget *);
void decrease_font_size(GtkWidget *, GtkWidget *);
void selection_changed(GtkWidget *, GtkWidget *);
gboolean window_state_event(GtkWidget *, GdkEventWindowState *, GtkWidget *);
gboolean button_press(GtkWidget *, GdkEventButton *, Terminal *);
gboolean key_press(GtkWidget *, GdkEventKey *, Terminal *);
void new_window(Terminal *);
void copy_text(Terminal *);
void paste_text(Terminal *);
void fullscreen(Terminal *);
void config_file_changed(Config *);
void palette_changed(gchar *);
void open_url(GtkWidget *, char *);
void open_email(GtkWidget *, char *);

#endif /* CALLBACK_H */
