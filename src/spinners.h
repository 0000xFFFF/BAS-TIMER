#ifndef SPINNERS_H
#define SPINNERS_H

// Declare the Spinner struct
typedef struct {
    char** frames;   // Array of frames (strings)
    int frame_count; // Number of frames
    int index;       // Current frame index
} Spinner;

// Function prototypes
void init_spinner(Spinner* spinner, char** frames, int frame_count);
void spin_spinner(Spinner* spinner);
char* get_frame(Spinner* spinner, int also_spin);
void init_spinners();

// Declare external spinners
extern Spinner spinner_basic;
extern Spinner spinner_bars;
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

#endif // SPINNER_H
