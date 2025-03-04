#include "spinners.h"

// Function to initialize a spinner
void init_spinner(Spinner* spinner, char** frames, int frame_count) {
    spinner->frames = frames;
    spinner->frame_count = frame_count;
    spinner->index = 0;
}

void spin_spinner(Spinner* spinner) {
    spinner->index = (spinner->index + 1) % spinner->frame_count;
}

// Function to get the current frame
char* get_frame(Spinner* spinner, int also_spin) {
    char* frame = spinner->frames[spinner->index];
    if (also_spin) { spin_spinner(spinner); }
    return frame;
}

Spinner spinner_basic;
char* frames_spinner_basic[] = {"-", "\\", "|", "/"};
Spinner spinner_bars;
char* frames_spinner_bars[] = { "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁"};
// clock old: "🕛", "🕐", "🕑", "🕒", "🕓", "🕔", "🕕", "🕖", "🕗", "🕘", "🕙", "🕚"};
Spinner spinner_clock;
char* frames_spinner_clock[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
Spinner spinner_lights;
char* frames_spinner_lights[] = {"󱩎", "󱩏", "󱩐", "󱩑", "󱩒", "󱩓", "󱩔", "󱩕", "󱩖", "󰛨"};
Spinner spinner_check;
char* frames_spinner_check[] = {"", "", "󰄬", "", "", "󰄭", "󰸞", "󰡕"};
Spinner spinner_warn;
char* frames_spinner_warn[] = {"", ""};
Spinner spinner_snow;
char* frames_spinner_snow[] = {"", "󰜗", "", "󰼪"};
Spinner spinner_heat;
char* frames_spinner_heat[] = {"󰐸", "󰫗"};
Spinner spinner_heat_pump;
char* frames_spinner_heat_pump[] = {"󱩃", "󱩄"};
Spinner spinner_eye_left;
char* frames_spinner_eye_left[] = {"󰛐", "󱣾"};
Spinner spinner_eye_right;
char* frames_spinner_eye_right[] = {"󰛐", "󱤀"};
Spinner spinner_circle;
char* frames_spinner_circle[] = {"󰪞", "󰪟", "󰪠", "󰪡", "󰪢", "󰪣", "󰪤", "󰪥"};
Spinner spinner_solar;
char* frames_spinner_solar[] = {"󱩳", "󱩴"};
Spinner spinner_fire;
char* frames_spinner_fire[] = {"", "", "󰈸", ""};
Spinner spinner_lightning;
char* frames_spinner_lightning[] = {"󱐌", "󱐋"};
Spinner spinner_sunrise;
char* frames_spinner_sunrise[] = {"󰖚", "󰖜"};
Spinner spinner_sunset;
char* frames_spinner_sunset[] = {"󰖚", "󰖛"};

void init_spinners() {
    init_spinner(&spinner_basic, frames_spinner_basic, sizeof(frames_spinner_basic)/sizeof(frames_spinner_basic[0]));
    init_spinner(&spinner_bars, frames_spinner_bars, sizeof(frames_spinner_bars)/sizeof(frames_spinner_bars[0]));
    init_spinner(&spinner_clock, frames_spinner_clock, sizeof(frames_spinner_clock)/sizeof(frames_spinner_clock[0]));
    init_spinner(&spinner_lights, frames_spinner_lights, sizeof(frames_spinner_lights)/sizeof(frames_spinner_lights[0]));
    init_spinner(&spinner_check, frames_spinner_check, sizeof(frames_spinner_check)/sizeof(frames_spinner_check[0]));
    init_spinner(&spinner_warn, frames_spinner_warn, sizeof(frames_spinner_warn)/sizeof(frames_spinner_warn[0]));
    init_spinner(&spinner_snow, frames_spinner_snow, sizeof(frames_spinner_snow)/sizeof(frames_spinner_snow[0]));
    init_spinner(&spinner_heat, frames_spinner_heat, sizeof(frames_spinner_heat)/sizeof(frames_spinner_heat[0]));
    init_spinner(&spinner_heat_pump, frames_spinner_heat_pump, sizeof(frames_spinner_heat_pump)/sizeof(frames_spinner_heat_pump[0]));
    init_spinner(&spinner_eye_left, frames_spinner_eye_left, sizeof(frames_spinner_eye_left)/sizeof(frames_spinner_eye_left[0]));
    init_spinner(&spinner_eye_right, frames_spinner_eye_right, sizeof(frames_spinner_eye_right)/sizeof(frames_spinner_eye_right[0]));
    init_spinner(&spinner_circle, frames_spinner_circle, sizeof(frames_spinner_circle)/sizeof(frames_spinner_circle[0]));
    init_spinner(&spinner_solar, frames_spinner_solar, sizeof(frames_spinner_solar)/sizeof(frames_spinner_solar[0]));
    init_spinner(&spinner_fire, frames_spinner_fire, sizeof(frames_spinner_fire)/sizeof(frames_spinner_fire[0]));
    init_spinner(&spinner_lightning, frames_spinner_lightning, sizeof(frames_spinner_lightning)/sizeof(frames_spinner_lightning[0]));
    init_spinner(&spinner_sunrise, frames_spinner_sunrise, sizeof(frames_spinner_sunrise)/sizeof(frames_spinner_sunrise[0]));
    init_spinner(&spinner_sunset, frames_spinner_sunset, sizeof(frames_spinner_sunset)/sizeof(frames_spinner_sunset[0]));
}
