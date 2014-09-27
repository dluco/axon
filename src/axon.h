#ifndef AXON_H
#define AXON_H

#define URL_REGEX "([\\w-]+://?|www[.])[^\\s()<>]+(?:\\([\\w\\d]+\\)|([^[:punct:]\\s]|/))"
#define WORD_CHARS "-A-Za-z0-9,./?%&#:_=+@~"
#define DEFAULT_CONFIG_FILE "axonrc"
#define CFG_GROUP "axon"
#define DEFAULT_COLUMNS 80
#define DEFAULT_ROWS 24
#define DEFAULT_FONT "Monospace, 11"
#define PALETTE_SIZE 16
#define DEFAULT_COLOR_SCHEME "white-on-black"
#define DEFAULT_OPACITY 100
#define SCROLL_ON_OUTPUT FALSE
#define SCROLL_ON_KEYSTROKE TRUE
#define SCROLLBAR FALSE
#define SCROLLBACK_LINES 1024
#define ALLOW_BOLD TRUE
#define AUDIBLE_BELL FALSE
#define VISIBLE_BELL FALSE
#define BLINKING_CURSOR FALSE
#define DEFAULT_CURSOR_TYPE VTE_CURSOR_SHAPE_BLOCK
#define AUTOHIDE_MOUSE FALSE

enum
{
	TITLE_MODE_REPLACE,
	TITLE_MODE_IGNORE
};
#define DEFAULT_TITLE_MODE TITLE_MODE_REPLACE

#define NEW_WINDOW_ACCEL (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define COPY_ACCEL (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define PASTE_ACCEL (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define CLOSE_WINDOW_ACCEL (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define RESET_ACCEL (GDK_CONTROL_MASK | GDK_SHIFT_MASK)
#define NEW_WINDOW_KEY GDK_KEY_N
#define COPY_KEY GDK_KEY_C
#define PASTE_KEY GDK_KEY_V
#define CLOSE_WINDOW_KEY GDK_KEY_Q
#define RESET_KEY GDK_KEY_R
#define FULLSCREEN_KEY GDK_KEY_F11
#define MENU_KEY GDK_KEY_Menu

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

typedef struct _config {
	char *font;
	char *palette;
	int opacity;
	int title_mode;
	gboolean scroll_on_output;
	gboolean scroll_on_keystroke;
	gboolean show_scrollbar;
	int scrollback_lines;
	gboolean allow_bold;
	gboolean audible_bell;
	gboolean visible_bell;
	gboolean blinking_cursor;
	VteTerminalCursorShape cursor_type;
	gboolean autohide_mouse;
	char *word_chars;
	gboolean modified;
} Config;

typedef struct _terminal {
	GtkWidget *window;
	GtkWidget *menu;
	GtkWidget *hbox;
	GtkWidget *vte; /* VTE terminal */
	GtkWidget *scrollbar;
	Config *conf; /* associated Config */
	Options *opts; /* associated Options */
	GPid pid;
	gboolean fullscreen; /* fullscreen state */
} Terminal;


#endif /* AXON_H */
