#ifndef TERM_H
#define TERM_H

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

#endif // TERM_H
