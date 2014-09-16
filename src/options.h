#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct options {
	char *work_dir;
	char *execute;
	gchar **xterm_args;
	gboolean xterm_execute;
	gboolean version;
	gint login;
	char *title;
	gboolean hold;
	char *geometry;
	char *config_file;
	gboolean fullscreen;
	gboolean maximize;
} Options;

Options *options_new(void);
void options_free(Options *);
void options_parse(Options *, int, char *[]);

#endif /* OPTIONS_H */
