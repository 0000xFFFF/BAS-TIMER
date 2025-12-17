#include "main_utils.h"
#include "globals.h"
#include <float.h>
#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void mkdir_safe(const char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1) { mkdir(dir, 0700); }
}

void change_to_bin_dir(void)
{
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len != -1) {
        path[len] = '\0';          // Null-terminate the string
        char* dir = dirname(path); // Extract directory part

        if (chdir(dir) != 0) { perror("chdir failed"); }
    }
    else {
        perror("readlink failed");
    }
}

/*
** - handles quoted values like "hello world"
** - handles unquoted values like hello
** - trims whitespace
** - strips newline
** - works with spaces inside quotes
** - does not modify the original line buffer in unsafe ways
*/
int load_env(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    char line[BIGBUFF];

    while (fgets(line, sizeof(line), file)) {
        // Trim leading spaces
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        // Skip blank lines or comments
        if (*p == '#' || *p == '\n' || *p == '\0') continue;

        // Find '='
        char* eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = p;
        char* value = eq + 1;

        // Trim trailing whitespace from key
        for (char* end = key + strlen(key) - 1;
             end > key && (*end == ' ' || *end == '\t');
             end--) {
            *end = '\0';
        }

        // Trim newline and trailing spaces on value
        size_t len = strlen(value);
        while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == ' ' || value[len - 1] == '\t'))
            value[--len] = '\0';

        // Strip surrounding quotes if present
        if (value[0] == '"' && len >= 2 && value[len - 1] == '"') {
            value[len - 1] = '\0'; // remove ending quote
            value++;               // skip starting quote
        }

        setenv(key, value, 1);
    }

    fclose(file);
    return 1;
}

bool env_str_to_bool(const char* s)
{
    if (s == NULL) { return true; }

    if (strcmp(s, "false") == 0 || strcmp(s, "0") == 0 || strcmp(s, "no") == 0) { return false; }

    return true;
}
