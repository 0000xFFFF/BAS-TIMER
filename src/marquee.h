#ifndef MARQUEE_H
#define MARQUEE_H

#include <stddef.h>

typedef struct {
    const char* text;  // The text to scroll (ANSI + UTF-8 allowed)
    int width;         // Terminal width for scrolling
    int pos;           // Current scroll position in visible chars
    int text_len;      // Visible length (excluding ANSI)
    int scroll_needed; // True if scrolling required
    int i;             // Internal counter
    int scroll_on;     // Scroll every n calls
    char ansi_state[128]; // Current ANSI codes
} Marquee;

// Initialize a marquee
void init_marquee(Marquee* m, const char* text, int width, int scroll_on);

// Scroll the marquee (update internal position)
void scroll_marquee(Marquee* m);

// Render current frame into a buffer (does not print)
void render_marquee(Marquee* m, char* buffer, size_t buffer_size);

#endif // MARQUEE_H
