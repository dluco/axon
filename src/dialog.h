#ifndef DIALOG_H
#define DIALOG_H

void dialog_message(GtkWidget *, GtkMessageType, gchar *, ...);
gint dialog_message_question(GtkWidget *, gchar *, ...);

#endif /* DIALOG_H */
