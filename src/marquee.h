#ifndef MARQUEE_H
#define MARQUEE_H

#include <stddef.h>

typedef struct {
    const char* text;
    int width;
    int pos;              // Scroll position in visible chars
    int text_len;         // Visible length
    int scroll_needed;    // = term size < text_len
    int i;                // Iter for update_on
    int update_on;        // Spin every i frames
    int start_delay;      // wait a few marquee_scrolls before moving 0 pos
    char ansi_state[128]; // Active ANSI codes
} Marquee;

extern int marquee_visible_length(const char* str);
extern void marquee_init(Marquee* m, const char* text, int width, int start_delay, int update_on);
extern int marquee_render(Marquee* m, char* buffer, size_t size);
extern void marquee_scroll(Marquee* m);
extern void marquee_scroll_smart(Marquee* m);

#endif // MARQUEE_H
