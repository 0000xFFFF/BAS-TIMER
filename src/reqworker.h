#ifndef REQWORKER_H
#define REQWORKER_H

struct bas_info {

    int hasValues; // to check if struct is empty or not

    // statuses
    int mod_rada;
    int mod_rezim;
    int StatusPumpe3;
    int StatusPumpe4;
    int StatusPumpe5;
    int StatusPumpe6;
    int StatusPumpe7;

    // temps
    double Tspv;
    double Tsolar;
    double Tzadata;
    double Tfs;
    double Tmax;
    double Tmin;
    double Tsobna;

    // other calced values
    double Tmid;
    double Thottest;
    double Tcoldest;
    int TmidGE;
    int TminLT;
};

void init_reqworker();
void reqworker_do_work();
char* sendreq_error_to_str(int e);

#endif // REQWORKER_H
