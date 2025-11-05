#ifndef SPINNERS_H
#define SPINNERS_H

// Declare the Spinner struct
struct Spinner {
    char** frames;    // Array of frames (strings)
    int frame_count;  // Number of frames
    int index;        // Current frame index
    int i;            // Iter for update_on
    int update_on;    // Spin every i frames
};

// Function prototypes
extern void spin_spinner(struct Spinner*);
extern char* get_frame(struct Spinner*, int);
extern void init_spinners();

// Declare external spinners
extern struct Spinner spinner_basic;
extern struct Spinner spinner_bars;
extern struct Spinner spinner_bars_hori;
extern struct Spinner spinner_clock;
extern struct Spinner spinner_lights;
extern struct Spinner spinner_check;
extern struct Spinner spinner_warn;
extern struct Spinner spinner_snow;
extern struct Spinner spinner_heat;
extern struct Spinner spinner_heat_pump;
extern struct Spinner spinner_eye_left;
extern struct Spinner spinner_eye_right;
extern struct Spinner spinner_circle;
extern struct Spinner spinner_solar;
extern struct Spinner spinner_fire;
extern struct Spinner spinner_lightning;
extern struct Spinner spinner_sunrise;
extern struct Spinner spinner_sunset;
extern struct Spinner spinner_rain;
extern struct Spinner spinner_cloud;
extern struct Spinner spinner_sun;
extern struct Spinner spinner_thunder;
extern struct Spinner spinner_fog;
extern struct Spinner spinner_qm;

// slow emojis
extern struct Spinner spinner_window;
extern struct Spinner spinner_solar_panel;
extern struct Spinner spinner_cog;
extern struct Spinner spinner_house;
extern struct Spinner spinner_recycle;

#endif // SPINNER_H
