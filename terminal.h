#ifndef TERMINAL_H
#define TERMINAL_H

#include "config.h"

typedef struct terminal {
	GtkWidget *window;
	GtkWidget *menu;
	GtkWidget *hbox;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *scrollbar;
	/* state variables */
	gboolean fullscreen;
} Terminal;

Terminal *terminal_new(void);
void terminal_init(Terminal *);
void terminal_load_config(Terminal *, Config *);
void terminal_run(Terminal *, char *);

#endif /* TERMINAL_H */
