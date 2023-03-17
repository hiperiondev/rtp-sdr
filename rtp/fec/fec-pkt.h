/*
 * Copyright 2023 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/rtp-sdr *
 *
 * This is based on other projects:
 *    IDEA: https://github.com/OpenResearchInstitute/ka9q-sdr (not use any code of this)
 *    RTP: https://github.com/Daxbot/librtp/
 *    FEC: https://github.com/wesen/poc
 *    SOCKET: https://github.com/njh/mast
 *    Others: see individual files
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

#ifndef FEC_PKT_H__
#define FEC_PKT_H__

#include <sys/types.h>
#include <sys/socket.h>

#define FEC_PKT_MAX_GROUP_SEQ 255
#define FEC_PKT_MAX_PACKET_SEQ 255

/*M
 \emph{Structure representing a FEC packet header.}
 **/
typedef struct fec_pkt_hdr_s {
    unsigned char magic; /* 8 bits magic: 0xfe  */
    unsigned char version; /* version, default 1 */
    unsigned char group_seq; /* 8 bits group sequence number */
    unsigned char packet_seq; /* 8 bits packet sequence number */
    unsigned char fec_k; /* 8 bits FEC k parameter */
    unsigned char fec_n; /* 8 bits FEC n parameter */
    unsigned short fec_len; /* 16 bits FEC block length */
    unsigned short len; /* 16 bits payload length */
    unsigned long group_tstamp; /* 32 bits group timestamp in usecs */
} fec_pkt_hdr_t;

/*M
 \emph{Maximal size of a FEC packet.}
 **/
#define FEC_PKT_SIZE 65535

/*M
 \emph{Header size of a FEC packet header.}
 **/
#define FEC_PKT_HDR_SIZE 14

/*M
 \emph{Maximal FEC packet payload size.}
 **/
#define FEC_PKT_PAYLOAD_SIZE (FEC_PKT_SIZE - FEC_PKT_HDR_SIZE)

/*M
 \emph{FEC packet magic byte.}
 **/
#define FEC_PKT_MAGIC 0xfe

/*M
 \emph{Structure representing a FEC packet.}
 **/
typedef struct fec_pkt_s {
    fec_pkt_hdr_t hdr; /* packet header */
    unsigned char data[FEC_PKT_SIZE]; /* packet data: header + payload */
    unsigned char *payload; /* pointer to payload into data */
} fec_pkt_t;

/*M
 **/

void fec_pkt_init(/*@out@*/fec_pkt_t *pkt);

ssize_t fec_pkt_send(fec_pkt_t *pkt, int fd);
ssize_t fec_pkt_sendto(fec_pkt_t *pkt, int fd, struct sockaddr *to, socklen_t tolen);
int fec_pkt_read(fec_pkt_t *pkt, int fd);

#endif /* FEC_PKT_H__ */
