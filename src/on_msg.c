#include "onlfds.h"
#include "onevent.h"
#include "ontimer.h"

#include "on_msgagent.h"

#include "on_nat.h"

#include "on_msg.h"

static int __timer_msg_proc(onc_msg_s_t *msg)
{
    if (KKT_MSG_TIMER_EXPIRE == msg->msg) {
        onc_timerarg_s_t *arg =
            (onc_timerarg_s_t *)msg->arg;
        arg->timer_func(arg);
        return 0;
    }
    return -1;
}

static int __msg_proc(onc_msg_s_t *msg)
{
    onc_nat_msgproc(msg);
    __timer_msg_proc(msg);
    return 0;
}

int onc_msg_process(void *event, int msgbuf_pid, void *lfds)
{
    onc_msg_s_t msg;

    if (1 == onc_event_happen(event, lfds, KKT_EVENT_READ)) {
        while (0 == onc_msg_agent_recvmsg(msgbuf_pid, &msg)) {
            __msg_proc(&msg);
        }
        return 0;
    }

    return -1;
}
