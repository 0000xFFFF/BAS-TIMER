#include "../marquee.h"
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    setlocale(LC_ALL, "");

    struct Marquee m;
    int term_width = 20;
    char buffer[512];
    char somebuffer[3000] = "\033[31mHello 🌍! This is a \033[32mmarquee\033[0m scroll demo!";

    marquee_init(&m, somebuffer, term_width, 1, 1);

    while (1) {
        int len = marquee_render(&m, buffer, sizeof(buffer));
        if (len < 0) {
            fprintf(stderr, "Buffer overflow\n");
            break;
        }

        printf("\r%s\033[K", buffer);
        fflush(stdout);

        marquee_scroll(&m);
        usleep(200000);
    }

    return 0;
}
