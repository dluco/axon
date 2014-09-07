#ifndef DIALOG_H
#define DIALOG_H

void dialog_preferences(Terminal *);
void dialog_preferences_font(Terminal *);
void dialog_about(void);
gint dialog_message_question(GtkWidget *, gchar *, ...);

#endif /* DIALOG_H */
