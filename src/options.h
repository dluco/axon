#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct options {
	char *font;
	char *work_dir;
	char *execute;
	gchar **xterm_args;
	gboolean xterm_execute;
	gboolean version;
	gboolean colortable;
	gint login;
	char *title;
	int rows, columns;
	gboolean hold;
	char *geometry;
	char *config_file;
	char *output_file;
	gboolean fullscreen;
	gboolean maximize;
} Options;

Options *options_new(void);
void options_free(Options *);
void options_parse(Options *, int, char *[]);

#endif /* OPTIONS_H */
