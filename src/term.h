#ifndef TERM_H
#define TERM_H

#include <stddef.h>

extern int g_term_w;
extern int g_term_h;
extern void term_clear();
extern void term_cursor_reset();
extern void term_cursor_hide();
extern void term_cursor_show();
extern void term_blank();
extern int term_width();
extern int term_height();
extern void term_init();
extern int utf8_display_width(const char* s);
extern size_t ansi_to_html(const char* text, char* result);
extern void escape_quotes(const char* input, char* output);

#endif // TERM_H
