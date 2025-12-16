#include "thread_restarter.h"
#include "debug.h"
#include "globals.h"
#include "logger.h"
#include "request.h"
#include "thread_utils.h"
#include "utils.h"
#include <linux/reboot.h>
#include <stdatomic.h>
#include <sys/reboot.h>

static void if_unhealthy_restart(void)
{
    enum RequestStatus bas_status = infos_bas_health();

    // if everything is failing restart
    if ((!is_connection_healthy() || !can_get_local_ips()) && request_status_failed(bas_status)) {
        printf("Rebooting system...\n");
        logger_write_changes("system - reboot\n");

        sync(); // Sync filesystems before reboot

        // Perform reboot
        if (reboot(LINUX_REBOOT_CMD_RESTART) != 0) {
            perror("reboot");
        }
    }
}

void* th_restarter(void* sig)
{
    if (!ENABLE_RESTARTER) { return NULL; }

    DPL("THREAD START RESTARTER");
    UNUSED(sig);

    while (atomic_load(&g_running)) {
        sleep_ms_interruptible(SLEEP_MS_RESTARTER);
        if_unhealthy_restart();
    }

    DPL("THREAD STOP RESTARTER");
    return NULL;
}
