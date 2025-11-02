#define _XOPEN_SOURCE 700 // needed for wcwidth
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

int main() {
    setlocale(LC_ALL, "");  // Important for UTF-8 handling

    //const char *s = "󰛐󰙇";
    const char *s = "󱤀󱪯 changed to: 600";
    mbstate_t ps = {0};
    const char *p = s;
    wchar_t wc;
    size_t n;
    int width = 0;

    while ((n = mbrtowc(&wc, p, MB_CUR_MAX, &ps)) > 0) {
        int w = wcwidth(wc);
        if (w > 0) width += w;   // ignore non-printing
        p += n;
    }

    printf("UTF-8 string: %s\n", s);
    printf("Bytes: %zu\n", strlen(s));
    printf("Display width (terminal cells): %d\n", width);

    return 0;
}
