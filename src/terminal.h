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
	Config *conf; /* associated Config */
	Options *opts; /* associated Options */
	GPid pid;
	/* state variables and widget "bookmarks" */
	gboolean fullscreen;
	GtkWidget *fullscreen_item;
} Terminal;

Terminal *terminal_initialize(void);
void terminal_load_config(Terminal *, Config *);
void terminal_load_options(Terminal *, Options *);
void terminal_set_font(Terminal *, char *);
void terminal_set_palette(Terminal *, char *);
void terminal_set_opacity(Terminal *, int);
void terminal_run(Terminal *, char *);
void terminal_show(Terminal *);
char *terminal_get_cwd(Terminal *);

#endif /* TERMINAL_H */
