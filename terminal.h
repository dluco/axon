#ifndef TERMINAL_H
#define TERMINAL_H

typedef struct terminal {
	GtkWidget *window;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *hbox;
	GtkWidget *scrollbar;
} Terminal;

Terminal *terminal_init(void);

#endif /* TERMINAL_H */
