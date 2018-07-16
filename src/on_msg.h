#ifndef __ONC_MSG____H__
#define __ONC_MSG____H__

#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

int     onc_msg_process(void *event, int msgbuf_pid, void *lfds);

#ifdef __cplusplus
}
#endif

#endif
