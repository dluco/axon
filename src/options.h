#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct _options {
	char *work_dir;
	char *execute;
	char **xterm_args;
	char *title;
	char *geometry;
	char *config_file;
	gboolean version;
	gboolean xterm_execute;
	gint login;
	gboolean fullscreen;
} Options;

Options *options_parse(int argc, char *argv[]);

#endif /* OPTIONS_H */
