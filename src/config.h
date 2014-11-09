#ifndef _CONFIG_H_
#define _CONFIG_H_

/* configuration defaults */
#define URL_REGEX "((ftp|http)s?://|www\\.)[-a-zA-Z0-9.?$%&/=_~#.,:;+]*"
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
#define HIGHLIGHT_URLS FALSE
#define AUDIBLE_BELL FALSE
#define VISIBLE_BELL FALSE
#define BLINKING_CURSOR FALSE
#define DEFAULT_CURSOR_TYPE VTE_CURSOR_SHAPE_BLOCK
#define AUTOHIDE_MOUSE FALSE
enum { TITLE_MODE_REPLACE, TITLE_MODE_IGNORE };
#define DEFAULT_TITLE_MODE TITLE_MODE_REPLACE

/* keyboard shortcuts */
#define MODKEY GDK_CONTROL_MASK

Key keys[] = {
	/* mask,					key,		function */
	{ MODKEY|GDK_SHIFT_MASK,	GDK_n,		terminal_new_window },
	{ MODKEY|GDK_SHIFT_MASK,	GDK_c,		terminal_copy_text },
	{ MODKEY|GDK_SHIFT_MASK,	GDK_v,		terminal_paste_text },
	{ MODKEY|GDK_SHIFT_MASK,	GDK_q,		terminal_destroy },
	{ MODKEY|GDK_SHIFT_MASK,	GDK_r,		terminal_reset },
	{ 0,						GDK_F11,	terminal_fullscreen },
	{ 0,						GDK_Menu,	terminal_menu_popup },
};

#endif /* _CONFIG_H_ */
