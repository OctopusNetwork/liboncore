#ifndef __KKT_NAT____H__
#define __KKT_NAT____H__

#include "on_iport.h"

#include "on_msgagent.h"

#ifdef __cplusplus
extern "C" {
#endif

int     onc_nat_detect(void *socket,
            void *timer_handle, int msg_listener);
int     onc_nat_msgproc(onc_msg_s_t *msg);
int     onc_nat_process(void *socket, char *buf, int len,
            onc_ip_t ip, onc_port_t port);

#ifdef __cplusplus
}
#endif

#endif
