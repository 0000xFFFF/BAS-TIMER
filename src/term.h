#ifndef TERM_H
#define TERM_H

#include <stddef.h>

extern int g_term_w;
extern int g_term_h;
extern void term_clear(void);
extern void term_cursor_reset(void);
extern void term_cursor_hide(void);
extern void term_cursor_show(void);
extern void term_blank(void);
extern unsigned short int term_width(void);
extern unsigned short int term_height(void);
extern void term_init(void);
extern int utf8_display_width(const char* s);
extern size_t ansi_to_html(const char* text, char* result);
extern void escape_quotes(const char* input, char* output);

#endif // TERM_H
