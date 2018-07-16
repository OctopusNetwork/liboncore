#ifndef __ONC_UDP____H__
#define __ONC_UDP____H__

#include <sys/select.h>

#ifdef __cplusplus
extern "C" {
#endif

int   onc_udp_process(void *socket, void *lfds);

#ifdef __cplusplus
}
#endif

#endif
