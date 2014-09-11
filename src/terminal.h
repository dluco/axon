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
	gint regex_tags[2];
	gboolean fullscreen;
	GtkWidget *fullscreen_item;
} Terminal;

Terminal *terminal_new(void);
void terminal_init(Terminal *);
void terminal_load_config(Terminal *, Config *);
void terminal_load_options(Terminal *, Options *);
void terminal_set_palette(Terminal *, char *);
void terminal_run(Terminal *, char *);
void terminal_set_font(Terminal *, char *);
void terminal_show(Terminal *);
char *terminal_get_cwd(Terminal *);

#endif /* TERMINAL_H */
