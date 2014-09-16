#ifndef CONFIG_H
#define CONFIG_H

#define URL_REGEX "([\\w-]+://?|www[.])[^\\s()<>]+(?:\\([\\w\\d]+\\)|([^[:punct:]\\s]|/))"
#define DEFAULT_WORD_CHARS "-A-Za-z0-9,./?%&#:_=+@~"
#define DEFAULT_CONFIG_FILE "axonrc"
#define CFG_GROUP "axon"
#define DEFAULT_COLUMNS 80
#define DEFAULT_ROWS 24
#define DEFAULT_FONT "Monospace, 11"
#define PALETTE_SIZE 16
#define DEFAULT_COLOR_SCHEME "white-on-black"
#define DEFAULT_OPACITY 100
#define DEFAULT_TITLE_MODE "replace"
#define SCROLL_ON_OUTPUT FALSE
#define SCROLL_ON_KEYSTROKE TRUE
#define SCROLLBAR FALSE
#define SCROLLBACK_LINES 1024
#define ALLOW_BOLD TRUE
#define AUDIBLE_BELL FALSE
#define VISIBLE_BELL FALSE
#define BLINKING_CURSOR FALSE
#define DEFAULT_CURSOR_TYPE "block"
#define AUTOHIDE_MOUSE FALSE

enum
{
	TITLE_MODE_REPLACE,
	TITLE_MODE_IGNORE
};

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

typedef struct config {
	GKeyFile *cfg;
	char *config_file;
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
	gboolean modified_externally; /* modified by another process */
} Config;

Config *config_new(void);
void config_init(Config *);
void config_set_integer(Config *, const char *, int);
void config_set_value(Config *, const char *, const char *);
void config_set_boolean(Config *, const char *, gboolean);
void config_load(Config *, char *);
void config_save(Config *, GtkWidget *);
void config_free(Config *);

#endif /* CONFIG_H */
