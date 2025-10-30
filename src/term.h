#ifndef TERM_H
#define TERM_H

extern void term_clear();
extern void term_cursor_reset();
extern void term_cursor_hide();
extern void term_cursor_show();
extern void term_blank();
extern int term_width();

#endif // TERM_H
