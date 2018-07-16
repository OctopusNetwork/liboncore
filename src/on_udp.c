#include "kkt_iport.h"

#include "udp_socket.h"

#include "kkt_nat.h"

#include "kkt_udp.h"

int kkt_udp_process(void *socket, void *lfds)
{
#define KKT_MAX_RECV_BYTES  4096
    int recv_result = 0;
    char buf[KKT_MAX_RECV_BYTES];
    kkt_ip_t ip;
    kkt_port_t port;
    int rc = 0;

    if (1 == udp_socket_readable(socket, lfds)) {
        recv_result = udp_socket_recvfrom(socket, buf,
                KKT_MAX_RECV_BYTES, &ip, &port);
        if (0 < recv_result) {
            rc = kkt_nat_process(socket, buf,
                    recv_result, ip, port);
            if (0 == rc) {
                return rc;
            }
            return -1;
        }
    }

    return -1;
}
