/*
 * Copyright 2023 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/rtp-sdr *
 *
 * This is based on other projects:
 *      IDEA: https://github.com/OpenResearchInstitute/ka9q-sdr (not use any code of this)
 *       RTP: https://github.com/Daxbot/librtp/
 *       FEC: https://github.com/wesen/poc
 *    SOCKET: https://github.com/njh/mast
 *    OTHERS: see individual files
 *
 *    please contact their authors for more information.
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef RTP_SOCKET_H_
#define RTP_SOCKET_H_

#include <sys/socket.h>
#include <netinet/in.h>

//#define LOG

enum {
    DO_BIND_SOCKET,
    DONT_BIND_SOCKET
};

typedef enum {
    rtp_socket_LOG_DEBUG,
    rtp_socket_LOG_INFO,
    rtp_socket_LOG_WARN,
    rtp_socket_LOG_ERROR
} rtp_socket_log_level;

void rtp_socket_log(rtp_socket_log_level level, const char *fmt, ...);
#define rtp_socket_debug( ... ) rtp_socket_log(rtp_socket_LOG_DEBUG, __VA_ARGS__ )
#define rtp_socket_info( ... )  rtp_socket_log(rtp_socket_LOG_INFO, __VA_ARGS__ )
#define rtp_socket_warn( ... )  rtp_socket_log(rtp_socket_LOG_WARN, __VA_ARGS__ )
#define rtp_socket_error( ... ) rtp_socket_log(rtp_socket_LOG_ERROR, __VA_ARGS__ )

typedef struct rtp_socket_s {
             int fd;
             int joined_group;
    unsigned int if_index;

    struct sockaddr_storage dest_addr;
    struct sockaddr_storage src_addr;

    union {
        struct ipv6_mreq imr6;
          struct ip_mreq imr;
    };
} rtp_socket_t;

 int rtp_socket_open_recv(rtp_socket_t *sock, const char *address, const char *port, const char *ifname);
 int rtp_socket_open_send(rtp_socket_t *sock, const char *address, const char *port, const char *ifname);
 int rtp_socket_recv(rtp_socket_t *sock, void *data, unsigned int len);
 int rtp_socket_send(rtp_socket_t *sock, void *data, unsigned int len);
void rtp_socket_close(rtp_socket_t *sock);

#endif /* RTP_SOCKET_H_ */
