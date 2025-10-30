#include <stdio.h>

#include "../logger.h"

int main() {

    char buffer[1024] = {0};
    logger_changes_sumtime(buffer, 1024, "StatusPumpe4 = 0 -- ");
    printf("%s\n", buffer);

    return 0;
}
