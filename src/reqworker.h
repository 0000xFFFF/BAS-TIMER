#ifndef REQWORKER_H
#define REQWORKER_H

extern const char* URL_VARS;
extern const char* URL_HEAT_OFF;
extern const char* URL_HEAT_ON;
extern const char* URL_GAS_OFF;
extern const char* URL_GAS_ON;

extern char g_wttrin_buffer[];

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

extern int sendreq(const char* url, int log, int remember_response);
extern void init_reqworker();
extern void reqworker_do_work();
extern char* sendreq_error_to_str(int e);

#endif // REQWORKER_H
