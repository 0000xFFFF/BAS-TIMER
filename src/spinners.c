#include "spinners.h"
#include "globals.h"
#include <unistd.h>

// Function to initialize a spinner
static void init_spinner(struct Spinner* spinner, char** frames, int frame_count, int spin_on)
{
    spinner->frames = frames;
    spinner->frame_count = frame_count;
    spinner->index = 0;
    spinner->i = 0;
    spinner->update_on = spin_on;
}

void spin_spinner(struct Spinner* spinner)
{
    spinner->i++;
    if (spinner->i >= spinner->update_on) {
        spinner->i = 0;
        spinner->index = (spinner->index + 1) % spinner->frame_count;
    }
}

// Function to get the current frame
char* get_frame(struct Spinner* spinner, int also_spin)
{
    char* frame = spinner->frames[(int)spinner->index];
    if (also_spin) { spin_spinner(spinner); }
    return frame;
}

struct Spinner spinner_basic;
char* spinner_basic_frames[] = {"-", "\\", "|", "/"};
struct Spinner spinner_bars;
char* spinner_bars_frames[] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁"};
// clock old: "🕛", "🕐", "🕑", "🕒", "🕓", "🕔", "🕕", "🕖", "🕗", "🕘", "🕙", "🕚"};
struct Spinner spinner_bars_hori;
char* spinner_bars_hori_frames[] = { "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█", "▉", "▊", "▋", "▌", "▍", "▎", "▏" };
struct Spinner spinner_clock;
char* spinner_clock_frames[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
struct Spinner spinner_lights;
char* spinner_lights_frames[] = {"󱩎", "󱩏", "󱩐", "󱩑", "󱩒", "󱩓", "󱩔", "󱩕", "󱩖", "󰛨"};
struct Spinner spinner_check;
char* spinner_check_frames[] = {"", "", "󰄬", "", "", "󰄭", "󰸞", "󰡕"};
struct Spinner spinner_warn;
char* spinner_warn_frames[] = {"", ""};
struct Spinner spinner_snow;
char* spinner_snow_frames[] = {"", "󰜗", "", "󰼪"};
struct Spinner spinner_heat;
char* spinner_heat_frames[] = {"󰐸", "󰫗"};
struct Spinner spinner_heat_pump;
char* spinner_heat_pump_frames[] = {"󱩃", "󱩄"};
struct Spinner spinner_eye_left;
char* spinner_eye_left_frames[] = {"󰛐", "󱣾"};
struct Spinner spinner_eye_right;
char* spinner_eye_right_frames[] = {"󰛐", "󱤀"};
struct Spinner spinner_circle;
char* spinner_circle_frames[] = {"󰪞", "󰪟", "󰪠", "󰪡", "󰪢", "󰪣", "󰪤", "󰪥"};
struct Spinner spinner_solar;
char* spinner_solar_frames[] = {"󱩳", "󱩴"};
struct Spinner spinner_fire;
char* spinner_fire_frames[] = {"", "", "󰈸", ""};
struct Spinner spinner_lightning;
char* spinner_lightning_frames[] = {"󱐌", "󱐋"};
struct Spinner spinner_sunrise;
char* spinner_sunrise_frames[] = {"󰖚", "󰖜"};
struct Spinner spinner_sunset;
char* spinner_sunset_frames[] = {"󰖚", "󰖛"};

// weather
struct Spinner spinner_qm;
char* spinner_qm_frames[] = {"", ""};
struct Spinner spinner_sun;
char* spinner_sun_frames[] = {"", "", "", "󰖙"};
struct Spinner spinner_cloud;
char* spinner_cloud_frames[] = {"", "󰅟", ""};
struct Spinner spinner_fog;
char* spinner_fog_frames[] = {"", "󰖑"};
struct Spinner spinner_rain;
char* spinner_rain_frames[] = {"", "", "", "", ""};
struct Spinner spinner_thunder;
char* spinner_thunder_frames[] = {"󰖓", "󰖐"};

// SLOW EMOJI
struct Spinner spinner_window;
char* spinner_window_frames[] = {"󱇜", "󱇛"};
struct Spinner spinner_solar_panel;
char* spinner_solar_panel_frames[] = {"", "󰶛"};
struct Spinner spinner_cog;
char* spinner_cog_frames[] = {"󰒓", "󰢻"};
struct Spinner spinner_house;
char* spinner_house_frames[] = {"", ""};
struct Spinner spinner_recycle;
char* spinner_recycle_frames[] = {"", "󰑌", "󱎝"};

void init_spinners(void)
{
    int default_spinner_delay = 250 / SLEEP_MS_DRAW;
    init_spinner(&spinner_basic, spinner_basic_frames, sizeof(spinner_basic_frames) / sizeof(spinner_basic_frames[0]), default_spinner_delay);
    init_spinner(&spinner_bars, spinner_bars_frames, sizeof(spinner_bars_frames) / sizeof(spinner_bars_frames[0]), default_spinner_delay);
    init_spinner(&spinner_bars_hori, spinner_bars_hori_frames, sizeof(spinner_bars_hori_frames) / sizeof(spinner_bars_hori_frames[0]), default_spinner_delay);
    init_spinner(&spinner_clock, spinner_clock_frames, sizeof(spinner_clock_frames) / sizeof(spinner_clock_frames[0]), default_spinner_delay);
    init_spinner(&spinner_lights, spinner_lights_frames, sizeof(spinner_lights_frames) / sizeof(spinner_lights_frames[0]), default_spinner_delay);
    init_spinner(&spinner_check, spinner_check_frames, sizeof(spinner_check_frames) / sizeof(spinner_check_frames[0]), default_spinner_delay);
    init_spinner(&spinner_warn, spinner_warn_frames, sizeof(spinner_warn_frames) / sizeof(spinner_warn_frames[0]), default_spinner_delay);
    init_spinner(&spinner_snow, spinner_snow_frames, sizeof(spinner_snow_frames) / sizeof(spinner_snow_frames[0]), default_spinner_delay);
    init_spinner(&spinner_heat, spinner_heat_frames, sizeof(spinner_heat_frames) / sizeof(spinner_heat_frames[0]), default_spinner_delay);
    init_spinner(&spinner_heat_pump, spinner_heat_pump_frames, sizeof(spinner_heat_pump_frames) / sizeof(spinner_heat_pump_frames[0]), default_spinner_delay);
    init_spinner(&spinner_eye_left, spinner_eye_left_frames, sizeof(spinner_eye_left_frames) / sizeof(spinner_eye_left_frames[0]), default_spinner_delay);
    init_spinner(&spinner_eye_right, spinner_eye_right_frames, sizeof(spinner_eye_right_frames) / sizeof(spinner_eye_right_frames[0]), default_spinner_delay);
    init_spinner(&spinner_circle, spinner_circle_frames, sizeof(spinner_circle_frames) / sizeof(spinner_circle_frames[0]), default_spinner_delay);
    init_spinner(&spinner_solar, spinner_solar_frames, sizeof(spinner_solar_frames) / sizeof(spinner_solar_frames[0]), default_spinner_delay);
    init_spinner(&spinner_fire, spinner_fire_frames, sizeof(spinner_fire_frames) / sizeof(spinner_fire_frames[0]), default_spinner_delay);
    init_spinner(&spinner_lightning, spinner_lightning_frames, sizeof(spinner_lightning_frames) / sizeof(spinner_lightning_frames[0]), default_spinner_delay);
    init_spinner(&spinner_sunrise, spinner_sunrise_frames, sizeof(spinner_sunrise_frames) / sizeof(spinner_sunrise_frames[0]), default_spinner_delay);
    init_spinner(&spinner_sunset, spinner_sunset_frames, sizeof(spinner_sunset_frames) / sizeof(spinner_sunset_frames[0]), default_spinner_delay);

    // weather
    int weather_spinner_delay = 800 / SLEEP_MS_DRAW;
    init_spinner(&spinner_qm, spinner_qm_frames, sizeof(spinner_qm_frames) / sizeof(spinner_qm_frames[0]), weather_spinner_delay);
    init_spinner(&spinner_sun, spinner_sun_frames, sizeof(spinner_sun_frames) / sizeof(spinner_sun_frames[0]), weather_spinner_delay);
    init_spinner(&spinner_cloud, spinner_cloud_frames, sizeof(spinner_cloud_frames) / sizeof(spinner_cloud_frames[0]), weather_spinner_delay);
    init_spinner(&spinner_fog, spinner_fog_frames, sizeof(spinner_fog_frames) / sizeof(spinner_fog_frames[0]), weather_spinner_delay);
    init_spinner(&spinner_rain, spinner_rain_frames, sizeof(spinner_rain_frames) / sizeof(spinner_rain_frames[0]), weather_spinner_delay);
    init_spinner(&spinner_thunder, spinner_thunder_frames, sizeof(spinner_thunder_frames) / sizeof(spinner_thunder_frames[0]), weather_spinner_delay);

    int slow_emoji_delay = 3000 / SLEEP_MS_DRAW;
    init_spinner(&spinner_window, spinner_window_frames, sizeof(spinner_window_frames) / sizeof(spinner_window_frames[0]), slow_emoji_delay);
    init_spinner(&spinner_solar_panel, spinner_solar_panel_frames, sizeof(spinner_solar_panel_frames) / sizeof(spinner_solar_panel_frames[0]), slow_emoji_delay);
    init_spinner(&spinner_cog, spinner_cog_frames, sizeof(spinner_cog_frames) / sizeof(spinner_cog_frames[0]), slow_emoji_delay);
    init_spinner(&spinner_house, spinner_house_frames, sizeof(spinner_house_frames) / sizeof(spinner_house_frames[0]), slow_emoji_delay);
    init_spinner(&spinner_recycle, spinner_recycle_frames, sizeof(spinner_recycle_frames) / sizeof(spinner_recycle_frames[0]), slow_emoji_delay);
}
