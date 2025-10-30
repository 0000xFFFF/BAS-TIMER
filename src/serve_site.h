#ifndef SERVE_SITE
#define SERVE_SITE

#include "mongoose.h"
extern void serve_site(struct mg_connection* c, int ev, void* ev_data);

#endif // SERVE_SITE
