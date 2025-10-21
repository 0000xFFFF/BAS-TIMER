#include <stdio.h>

#include "../logger.h"

int main() {

    char buffer[1024] = {0};
    logger_sumtime(buffer, 1024, "changes.log", "StatusPumpe4 = 0 -- ");
    printf("%s\n", buffer);

    return 0;
}
