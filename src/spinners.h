#ifndef SPINNERS_H
#define SPINNERS_H

// Declare the Spinner struct
typedef struct {
    char** frames;    // Array of frames (strings)
    int frame_count;  // Number of frames
    int index;        // Current frame index
    int i;            // 
    int spin_on;      // Spin every i frames
} Spinner;

// Function prototypes
extern void spin_spinner(Spinner*);
extern char* get_frame(Spinner*, int);
extern void init_spinners();

// Declare external spinners
extern Spinner spinner_basic;
extern Spinner spinner_bars;
extern Spinner spinner_bars_hori;
extern Spinner spinner_clock;
extern Spinner spinner_lights;
extern Spinner spinner_check;
extern Spinner spinner_warn;
extern Spinner spinner_snow;
extern Spinner spinner_heat;
extern Spinner spinner_heat_pump;
extern Spinner spinner_eye_left;
extern Spinner spinner_eye_right;
extern Spinner spinner_circle;
extern Spinner spinner_solar;
extern Spinner spinner_fire;
extern Spinner spinner_lightning;
extern Spinner spinner_sunrise;
extern Spinner spinner_sunset;
extern Spinner spinner_rain;
extern Spinner spinner_cloud;
extern Spinner spinner_sun;
extern Spinner spinner_thunder;
extern Spinner spinner_fog;
extern Spinner spinner_qm;

// slow emojis
extern Spinner spinner_window;
extern Spinner spinner_solar_panel;
extern Spinner spinner_cog;
extern Spinner spinner_house;
extern Spinner spinner_recycle;

#endif // SPINNER_H
