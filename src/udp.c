/*
 * Copyright 2016, Daniel Hilst Selli <danielhilst@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include <riemann/_config.h>
#include "riemann/udp.h"        /* This header will not be installed, so I use `"'. */

int riemann_udp_recv(riemann_client_t *cli, uint8_t *buf, size_t len, int flags, struct timeval *tout, ssize_t *recv_bytes)
{
        return -1;              /* Not implemented */
}

int riemann_udp_send(riemann_client_t *cli, uint8_t *buf, size_t len, int flags, struct timeval *tout)
{
        size_t bytes;
#ifdef RIEMANN_WITH_TIMEOUT
        fd_set fds;
        int rc;
#endif

        assert(cli);
        assert(buf);
        assert(len >= 0);

#ifdef RIEMANN_WITH_TIMEOUT
        FD_ZERO(&fds);
        FD_SET(cli->sock, &fds);
        rc = select(cli->sock + 1, NULL, &fds, NULL, tout);
        if (rc == -1) {
                return -2;
        } else if (rc == 0) {
                return -3;      /* Timeout */
        }
                
        if (FD_ISSET(cli->sock, &fds)) {
#endif
        bytes = sendto(cli->sock, buf, len, flags, cli->srv_addrinfo->ai_addr, cli->srv_addrinfo->ai_addrlen);
#ifdef RIEMANN_WITH_TIMEOUT
        } else { 
                return -4;      /* Should not happen  */
        }/* if (FD_ISSET(cli->sock, &fds)) { */
#endif       
        if (bytes == -1) {
                return -1;
        }
        return 0;
}
