#ifndef _AXON_H_
#define _AXON_H_

/* structs */
typedef struct _Options Options;
typedef struct _Config Config;
typedef struct _Key Key;
typedef struct _Terminal Terminal;

struct _Options {
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
};

struct _Config {
	char *font;
	char *palette;
	int opacity;
	int title_mode;
	gboolean scroll_on_output;
	gboolean scroll_on_keystroke;
	gboolean show_scrollbar;
	int scrollback_lines;
	gboolean allow_bold;
	gboolean highlight_urls;
	gboolean audible_bell;
	gboolean visible_bell;
	gboolean blinking_cursor;
	VteTerminalCursorShape cursor_type;
	gboolean autohide_mouse;
	char *word_chars;
};

struct _Key {
	int mask;
	int keyval;
	void (*func)(Terminal *term);
};

struct _Terminal {
	GtkWidget *window;
	GtkWidget *menu;
	GtkWidget *hbox;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *scrollbar;
	Config *conf; /* associated Config */
	Options *opts; /* associated Options */
	GPid pid;
	gboolean fullscreen; /* fullscreen state */
};

#endif /* _AXON_H_ */
