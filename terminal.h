#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct terminal {
	GtkWidget *window;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *hbox;
	GtkWidget *scrollbar;
} Terminal;

Terminal *terminal_new(void);
void terminal_init(Terminal *);

#endif /* TERMINAL_H */
