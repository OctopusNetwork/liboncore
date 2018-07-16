#include <stdlib.h>

#include "on_thread.h"
#include "on_string.h"
#include "on_malloc.h"
#include "on_iport.h"

#include "on_msgagent.h"
#include "ontimer.h"
#include "onlfds.h"
#include "onevent.h"
#include "onevgrp.h"
#include "udp_socket.h"

#include "on_config.h"

#include "on_nat.h"
#include "on_udp.h"
#include "on_msg.h"
#include "on_core.h"

/* How much tasks(threads) support */
#define ONC_CORE_MSGBUF_CNT     32
#define ONC_CORE_MSGBUF_SIZE    20000
#define ONC_CORE_EVGRP_SLOTS    32

#define ONC_CORE_DEFAULT_WAIT   1000

typedef enum {
    N_CORE_IDLE,
    N_CORE_INITIALIZING,
    N_CORE_INITIALIZED
} onc_corestat_e_t;

typedef struct {
    onc_corestat_e_t    state;
    void               *core_thread;
    char               *init_json;
} onc_core_launcher_s_t;

typedef struct {
    void       *timer_handle;
    int         msgbuf_pid;
    void       *event_group;

    void       *udp_sock;
    void       *event;
} onc_core_s_t;

static onc_core_launcher_s_t    g_core_launcher;
static onc_core_s_t             g_core;

static int __msg_feeder(onc_msg_s_t *msg)
{
    if (g_core.msgbuf_pid == msg->receiver_pid) {
        /* Consume message */
        onc_event_wakeup(g_core.event);
        onc_msg_agent_feedmsg_myself(g_core.msgbuf_pid, msg);
        return 0;
    }

    /* Not consume message */
    return -1;
}

static void *__core_thread(void *arg)
{
    onc_core_launcher_s_t *cl =
        (onc_core_launcher_s_t *)arg;
    int rc = -1;
    int wait = ONC_CORE_DEFAULT_WAIT;
    void *lfds = NULL;

    rc = onc_config_init(cl->init_json);
    if (rc < 0) {
        return NULL;
    }

    rc = onc_msg_agent_init(ONC_CORE_MSGBUF_CNT, 1);
    if (rc < 0) {
        goto L_ERROR_MSGAGENT_INIT;
    }

    g_core.timer_handle = onc_timer_init(1);
    if (NULL == g_core.timer_handle) {
        goto L_ERROR_TIMER_INIT;
    }

    g_core.event_group = onc_evgrp_create(ONC_CORE_EVGRP_SLOTS);
    if (NULL == g_core.event_group) {
        goto L_ERROR_EVGRP_CREATE;
    }

    g_core.msgbuf_pid = onc_msg_agent_create_bidirect_buf(
            (unsigned long)cl->core_thread,
            ONC_CORE_MSGBUF_SIZE, ONC_CORE_MSGBUF_SIZE,
            __msg_feeder);
    if (g_core.msgbuf_pid < 0) {
        goto L_ERROR_MSGBUF_CREATE;
    }

    g_core.udp_sock = udp_socket_new(
            0, onc_config_get_udp_port());
    if (NULL == g_core.udp_sock) {
        goto L_ERROR_UDPSOCK_NEW;
    }

    g_core.event = onc_event_create(1,
            ONC_EVENT_READ | ONC_EVENT_ERROR,
            1, 0, 0);
    if (NULL == g_core.event) {
        goto L_ERROR_EVENT_CREATE;
    }

    rc = onc_evgrp_event_add(g_core.event_group, g_core.event);
    if (rc < 0) {
        goto L_ERROR_COREEVT_ADD;
    }

    rc = udp_socket_event_enroll(g_core.udp_sock, g_core.event_group);
    if (rc < 0) {
        goto L_ERROR_UDPSOCK_ENROLL;
    }

    rc = onc_msg_agent_start();
    if (rc < 0) {
        goto L_ERROR_MSGAGENT_START;
    }

    rc = onc_nat_detect(g_core.udp_sock,
            g_core.timer_handle, g_core.msgbuf_pid);
    if (rc < 0) {
        goto L_ERROR_NAT_DETECT;
    }

    lfds = onc_lfds_new();
    if (NULL == lfds) {
        goto L_ERROR_LFDS_NEW;
    }

    cl->state = N_CORE_INITIALIZED;

    do {
        rc = onc_evgrp_wait(g_core.event_group, wait, lfds);
        if (rc < 0) {

        } else if (0 < rc) {
            rc = onc_udp_process(g_core.udp_sock, lfds);
            if (0 == rc) {
                continue;
            }
            rc = onc_msg_process(g_core.event,
                    g_core.msgbuf_pid, lfds);
            if (0 == rc) {
                continue;
            }
        } else {

        }
    } while (N_CORE_IDLE != cl->state);

    onc_lfds_del(lfds);

L_ERROR_LFDS_NEW:
L_ERROR_NAT_DETECT:
    onc_msg_agent_stop();
L_ERROR_MSGAGENT_START:
    udp_socket_event_del(g_core.udp_sock, g_core.event_group);
L_ERROR_UDPSOCK_ENROLL:
    onc_event_destroy(g_core.event);
L_ERROR_EVENT_CREATE:
    onc_evgrp_event_del(g_core.event_group, g_core.event);
L_ERROR_COREEVT_ADD:
    udp_socket_del(g_core.udp_sock);
L_ERROR_UDPSOCK_NEW:
    onc_msg_agent_destroy_bidirect_buf(g_core.msgbuf_pid);
L_ERROR_MSGBUF_CREATE:
    onc_evgrp_destroy(g_core.event_group);
L_ERROR_EVGRP_CREATE:
    onc_timer_final(g_core.timer_handle);
L_ERROR_TIMER_INIT:
    onc_msg_agent_final();
L_ERROR_MSGAGENT_INIT:
    onc_config_final();
    return NULL;
}

static void __core_exit(void)
{
    onc_evgrp_wakeup(g_core.event_group);
}

int onc_core_init(char *init_json)
{
    int len = onc_strlen(init_json) + 1;

    if (N_CORE_IDLE != g_core_launcher.state) {
        return 0;
    }

    g_core_launcher.init_json = onc_malloc(len);
    onc_memset(g_core_launcher.init_json, 0x0, len);
    onc_strncpy(g_core_launcher.init_json, init_json, len);

    g_core_launcher.state = N_CORE_INITIALIZING;
    g_core_launcher.core_thread = onc_thread_create(
            __core_thread, &g_core_launcher);
    if (NULL == g_core_launcher.core_thread) {
        goto L_ERROR_CORE_THREAD_CREATE;
    }

    return 0;

L_ERROR_CORE_THREAD_CREATE:
    onc_free(g_core_launcher.init_json);
    g_core_launcher.state = N_CORE_IDLE;
    return -1;
}

void onc_core_final(void)
{
    g_core_launcher.state = N_CORE_IDLE;
    __core_exit();
    onc_thread_join(g_core_launcher.core_thread);
    onc_free(g_core_launcher.init_json);
}
