#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct _options {
	char *work_dir;
	char *command;
	char **execute_args;
	char *title;
	char *geometry;
	char *config_file;
	gboolean version;
	gboolean execute;
	gint login;
	gboolean fullscreen;
} Options;

Options *options_parse(int argc, char *argv[]);

#endif /* OPTIONS_H */
