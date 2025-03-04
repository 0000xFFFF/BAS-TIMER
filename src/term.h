#ifndef TERM_H
#define TERM_H

void term_clear();
void term_cursor_reset();
void term_cursor_hide();
void term_cursor_show();
void term_blank();
int ansi_to_html(const char* text, char* buffer);

#endif // TERM_H
