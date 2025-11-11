#ifndef SERVE_H
#define SERVE_H

#include "mongoose.h"
extern void serve(struct mg_connection* c, int ev, void* ev_data);

#endif // SERVE_H
