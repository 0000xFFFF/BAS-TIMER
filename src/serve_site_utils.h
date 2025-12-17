#ifndef SERVE_SITE_UTILS_H
#define SERVE_SITE_UTILS_H

#include "mongoose.h"

extern int mg_str_contains(struct mg_str haystack, const char* needle);

extern int serve_site_handle_route(struct mg_connection* c, struct mg_http_message* hm);

#endif // SERVE_SITE_UTILS_H
