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

#include "fec-pkt.h"

/*M
 \emph{FEC group structure.}

 Used to collect packets until enough are available to decode the group.
 **/
typedef struct fec_group_s {
    /*M
     $k$ FEC parameter. At least $k$ packets have to be collected in
     order to recover the complete source information.
     **/
    unsigned char fec_k;
    /*M
     $n$ FEC parameter. The group contains at least $n$ packets.
     **/
    unsigned char fec_n;
    /*M
     The maximal length of a packet. Packets with sequence numbers <
     $k$ can be shorter.
     **/
    unsigned short fec_len;
    /*M
     The group sequence number.
     **/
    unsigned char seq;
    /*M
     The group timestamp in usecs.
     **/
    unsigned long tstamp;
    /*M
     Received packets count.
     **/
    unsigned char rcvd_pkts;

    /*M
     Keeps track of received packets.
     **/
    unsigned char *pkts;
    /*M
     Buffer to be filled with packet payloads.
     **/
    unsigned char *buf;

    /* Length of the inserted packets. */
    unsigned int *lengths;

    int decoded;
} fec_group_t;

void fec_group_init(fec_group_t *group, unsigned char fec_k, unsigned char fec_n, unsigned char seq, unsigned long tstamp, unsigned short fec_len);
void fec_group_destroy(fec_group_t *group);
void fec_group_clear(fec_group_t *group);
void fec_group_insert_pkt(fec_group_t *group, fec_pkt_t *pkt);
int fec_group_decode(fec_group_t *group);
//int fec_group_decode_to_adus(fec_group_t *group, aq_t *aq);
