
#include "src/mongoose.h"
#include <float.h>

double extract_json_label(struct mg_str json_body, const char* label)
{
    struct mg_str tok = mg_json_get_tok(json_body, label);
    double value = DBL_MIN;
    mg_json_get_num(tok, "$.value", &value);
    return value;
}
