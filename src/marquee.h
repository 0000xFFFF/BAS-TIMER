#ifndef MARQUEE_H
#define MARQUEE_H

#include <stddef.h>

typedef struct {
    const char* text;
    int width;
    int pos;      // Scroll position in visible chars
    int text_len; // Visible length
    int scroll_needed;
    char ansi_state[128]; // Active ANSI codes
} Marquee;


extern int marquee_visible_length(const char* str);
extern void marquee_init(Marquee* m, const char* text, int width);
extern int marquee_render(Marquee* m, char* buffer, size_t size);
extern void marquee_scroll(Marquee* m);

#endif // MARQUEE_H
