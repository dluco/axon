#ifndef TERMINAL_H
#define TERMINAL_H

#include "config.h"
#include "options.h"

typedef struct terminal {
	GtkWidget *window;
	GtkWidget *menu;
	GtkWidget *hbox;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *scrollbar;
	char *match; /* matched regex string */
	Config *conf; /* associated Config */
	Options *opts; /* associated Options */
	GPid pid;
	/* state variables and widget "bookmarks" */
	gboolean fullscreen;
	GtkWidget *fullscreen_item;
} Terminal;

Terminal *terminal_new(void);
void terminal_init(Terminal *);
void terminal_load_config(Terminal *, Config *);
void terminal_load_options(Terminal *, Options *);
void terminal_load_color_scheme(Terminal *, const char *);
void terminal_run(Terminal *);

#endif /* TERMINAL_H */
