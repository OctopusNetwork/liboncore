#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "on_core.h"

static int  g_running = 0;

static void __sigint_handler(int sig)
{
    g_running = 0;
}

int main(int argc, char *argv[])
{
    if (on_core_init("{}")) {
        return -1;
    }

    g_running = 1;
    signal(SIGINT, __sigint_handler);

    do {
        sleep(1);
    } while (1 == g_running);

    on_core_final();

    return 0;
}
