#ifndef MARQUEE_H
#define MARQUEE_H

#include <stdbool.h>
#include <stddef.h>

#define MARQUEE_ZERO_WIDTH_SPACE "\xE2\x80\x8B"

struct Marquee {
    bool valid;
    const char* text;
    int width;
    int pos;              // Scroll position in visible chars
    int text_len;         // Visible length
    int scroll_needed;    // = term size < text_len
    int i;                // Iter for update_on
    int update_on;        // Spin every i frames
    int start_delay;      // wait a few marquee_scrolls before moving 0 pos
    char ansi_state[128]; // Active ANSI codes
};

extern int marquee_visible_length(const char* str);
extern void marquee_init(struct Marquee* m, const char* text, int width, int start_delay, int update_on);
extern void marquee_update_width(struct Marquee* m, int width);
extern int marquee_render(struct Marquee* m, char* buffer, size_t size);
extern void marquee_scroll(struct Marquee* m);
extern void marquee_scroll_smart(struct Marquee* m);

#endif // MARQUEE_H
