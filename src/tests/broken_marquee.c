#include "../marquee.h"
#include "../request.h"
#include "../colors.h"
#include "../debug.h"

#include <locale.h>
#include <stdio.h>
#include <unistd.h>

#define MZWS MARQUEE_ZERO_WIDTH_SPACE

static char g_temp[BIGBUFF];

static char* mk_str(const char* format, char* param)
{
    snprintf(g_temp, sizeof(g_temp), format, param);
    return g_temp;
}

int main()
{
    setlocale(LC_ALL, "");

    struct Marquee m;

    char m_buff[1024];

    size_t b = 0;
    // clang-format on
    b += ctext_fg(m_buff + b, sizeof(m_buff) - b, timeofday_to_color(TIME_OF_DAY_DAWN), mk_str(MZWS "🌄%s", "11:11:11"));
    b += ctext_fg(m_buff + b, sizeof(m_buff) - b, timeofday_to_color(TIME_OF_DAY_MORNING), mk_str(MZWS "🌅%s", "22:22:22"));
    b += ctext_fg(m_buff + b, sizeof(m_buff) - b, timeofday_to_color(TIME_OF_DAY_ZENITH), mk_str(MZWS "🌞%s", "33:33:33"));
    b += ctext_fg(m_buff + b, sizeof(m_buff) - b, timeofday_to_color(TIME_OF_DAY_SUNSET), mk_str(MZWS "🌇%s", "44:44:44"));
    b += ctext_fg(m_buff + b, sizeof(m_buff) - b, timeofday_to_color(TIME_OF_DAY_NIGHT), mk_str(MZWS "🌆%s", "55:55:55"));

    // clang-format off
    marquee_init(&m, m_buff, 20, 0, 1);

    char render_buff[1024];

    size_t new_len = strlen(m_buff);
    size_t m_buff_l = new_len;
    printf("len: %zu\n", new_len);

    while (1) {

        if (m_buff_l != new_len) {
            DPL("\n\n\nLEN CHANGED");
            printf("len: %zu\n", new_len);
            asm("int3");
        }

        int len = marquee_render(&m, render_buff, sizeof(render_buff));
        if (len < 0) {
            fprintf(stderr, "Buffer overflow\n");
            break;
        }

        printf("\r%s\033[K", render_buff);
        fflush(stdout);

        marquee_scroll(&m);
        //usleep(200000);
        //usleep(1);
    }

    return 0;
}
