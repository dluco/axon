#ifndef CONFIG_H
#define CONFIG_H

#define HTTP_REGEX "(((ftp|http)s?://)|(www|ftp)[-A-Za-z0-9]*\\.)[-A-Za-z0-9\\.]+(:[0-9]*)?"
#define DEFAULT_WORD_CHARS "-A-Za-z0-9,./?%&#:_=+@~"
#define DEFAULT_CONFIG_FILE PACKAGE"rc"
#define CFG_GROUP PACKAGE
#define DEFAULT_COLUMNS 80
#define DEFAULT_ROWS 24
#define DEFAULT_FONT "Monospace, 11"
#define SCROLL_ON_OUTPUT FALSE
#define SCROLL_ON_KEYSTROKE TRUE
#define SCROLLBAR FALSE
#define SCROLLBACK_LINES -1
#define AUDIBLE_BELL FALSE
#define VISIBLE_BELL FALSE
#define BLINKING_CURSOR FALSE

typedef struct config {
	GKeyFile *cfg;
	char *config_file;
	char *font;
	gboolean scroll_on_output;
	gboolean scroll_on_keystroke;
	gboolean show_scrollbar;
	int scrollback_lines;
	gboolean audible_bell;
	gboolean visible_bell;
	gboolean blinking_cursor;
	VteTerminalCursorShape cursor_type;
	char *word_chars;
	gboolean modified;
} Config;

Config *config_new(void);
void config_init(Config *);
void config_load(Config *);
void config_save(Config *);
void config_destroy(Config *);

#endif /* CONFIG_H */
