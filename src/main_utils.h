#ifndef MAIN_UTILS_HPP
#define MAIN_UTILS_HPP

#include <stdbool.h>

extern void mkdir_safe(const char* dir);
extern void change_to_bin_dir(void);
extern int load_env(const char* filename);
extern bool env_str_to_bool(const char* s);

#endif // MAIN_UTILS_HPP
