#include <stdio.h>
#include <stdlib.h>

#include "kkt_malloc.h"

#include "kkttimer.h"
#include "nat_detector.h"

#include "kkt_nat.h"

#define KKT_NAT_DETECT_TIMEOUT      4000
#define KKT_NAT_DETECT_RETRIES      3

typedef struct {
    void    *socket;
    int      msg_listener;
    int      retries;
} kkt_nat_timerarg_s_t;

static void __timer_detected(kkt_timerarg_s_t *arg);

static int __nat_detect(void *socket, void *timer_handle,
        int msg_listener, int retries)
{
    int rc = nat_detector_detect(socket,
            timer_handle, msg_listener, 0);
    if (0 == rc) {
        kkt_timerarg_s_t *nat_timer_arg =
            kkt_malloc(sizeof(kkt_timerarg_s_t));
        if (NULL == nat_timer_arg) {
            /* What can we do ? */
        } else {
            kkt_nat_timerarg_s_t *arg =
                kkt_malloc(sizeof(kkt_nat_timerarg_s_t));
            if (NULL == arg) {
                /* What can we do ? */
            } else {
                arg->socket = socket;
                arg->msg_listener = msg_listener;
                arg->retries = retries;
                nat_timer_arg->arg = arg;
                nat_timer_arg->timer_func = __timer_detected;
                if (NULL == kkt_timer_create(timer_handle,
                            msg_listener, 0, KKT_NAT_DETECT_TIMEOUT,
                            nat_timer_arg)) {
                    /* What can we do ? */
                }
            }
        }
    }

    return rc;
}

static void __timer_detected(kkt_timerarg_s_t *arg)
{
    kkt_nat_timerarg_s_t *nat_arg =
        (kkt_nat_timerarg_s_t *)arg->arg;
    int retries = nat_arg->retries;

    if (N_NOT_DETECTED == nat_detector_get_type()) {
        if (retries < KKT_NAT_DETECT_RETRIES) {
            retries++;
            __nat_detect(nat_arg->socket, arg->timer_handle,
                    nat_arg->msg_listener, retries);
        } else {
            nat_detector_timeout();
        }
    }
    kkt_timer_destroy(arg->timer_handle, arg->timer);
    kkt_free(arg->arg);
}

int kkt_nat_detect(void *socket,
        void *timer_handle, int msg_listener)
{
    return __nat_detect(socket, timer_handle,
            msg_listener, 0);
}

int kkt_nat_msgproc(kkt_msg_s_t *msg)
{
    switch (msg->msg) {
    case KKT_MSG_NAT_DETECTOR_DETECTED:
        printf("NAT detected\n");
        return 0;
    case KKT_MSG_NAT_DETECTOR_FAIL:
        printf("NAT detect fail\n");
        return 0;
    }
    return -1;
}

int kkt_nat_process(void *socket, char *buf, int len,
        kkt_ip_t ip, kkt_port_t port)
{
    return nat_detector_process(socket, buf, len, ip, port);
}
