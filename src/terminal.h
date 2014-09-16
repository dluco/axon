#ifndef TERMINAL_H
#define TERMINAL_H

#include "config.h"
#include "options.h"

typedef struct _terminal {
	GtkWidget *window;
	GtkWidget *menu;
	GtkWidget *hbox;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *scrollbar;
	Config *conf; /* associated Config */
	Options *opts; /* associated Options */
	GPid pid;
} Terminal;

Terminal *terminal_initialize(Config *conf, Options *opts);
void terminal_load_config(Terminal *term, Config *conf);
void terminal_load_options(Terminal *term, Options *opts);
void terminal_set_font(Terminal *term, char *font);
void terminal_set_palette(Terminal *term, char *palette_name);
void terminal_set_opacity(Terminal *term, int opacity);
void terminal_run(Terminal *term, Options *opts);
void terminal_show(Terminal *term);
char *terminal_get_cwd(Terminal *term);

#endif /* TERMINAL_H */
