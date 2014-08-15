#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_CONFIG_FILE PACKAGE"rc"
#define CFG_GROUP PACKAGE
#define DEFAULT_FONT "Ubuntu Mono,monospace 13"
#define SCROLL_ON_OUTPUT FALSE
#define SCROLL_ON_KEYSTROKE TRUE
#define SCROLLBAR TRUE
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
	gboolean fullscreen;
	gboolean modified;
} Config;

Config *config_new(void);
void config_init(Config *);
void config_load(Config *);
void config_save(Config *);
void config_destroy(Config *);

#endif /* CONFIG_H */
