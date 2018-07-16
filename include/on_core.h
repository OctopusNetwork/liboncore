#ifndef __ONC_CORE____H__
#define __ONC_CORE____H__

#ifdef __cplusplus
extern "C" {
#endif

int     onc_core_init(char *init_json);
void    onc_core_final(void);

#ifdef __cplusplus
}
#endif

#endif
