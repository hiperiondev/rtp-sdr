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

#ifndef FEC_GROUP_H_
#define FEC_GROUP_H_

#include "fec_pkt.h"

// FEC group structure.
// Used to collect packets until enough are available to decode the group.
typedef struct fec_group_s {
     unsigned char fec_k;     // k FEC parameter. At least k packets have to be collected in order to recover the complete source information.
     unsigned char fec_n;     // n FEC parameter. The group contains at least n packets.
    unsigned short fec_len;   // The maximal length of a packet. Packets with sequence numbers k can be shorter.
     unsigned char seq;       // The group sequence number.
     unsigned long tstamp;    // The group timestamp in usecs.
     unsigned char rcvd_pkts; // Received packets count.
     unsigned char *pkts;     // Keeps track of received packets.
     unsigned char *buf;      // Buffer to be filled with packet payloads.
      unsigned int *lengths;  // Length of the inserted packets.
               int decoded;
} fec_group_t;

void fec_group_init(fec_group_t *group, unsigned char fec_k, unsigned char fec_n, unsigned char seq, unsigned long tstamp, unsigned short fec_len);
void fec_group_destroy(fec_group_t *group);
void fec_group_clear(fec_group_t *group);
void fec_group_insert_pkt(fec_group_t *group, fec_pkt_t *pkt);
 int fec_group_decode(fec_group_t *group);

#endif // FEC_GROUP_H_
