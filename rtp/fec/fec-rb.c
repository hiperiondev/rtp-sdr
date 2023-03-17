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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fec-group.h"

/*M
 \emph{Maximal number of elements in the ring buffer.}
 **/
unsigned int fec_rb_size = 0;

/*M
 \emph{Index of first valid element in ring buffer.}
 **/
static unsigned int fec_rb_start = 0;

/*M
 \emph{Index of first invalid element in ring buffer.}

 **/
static unsigned int fec_rb_end = 0;

/*M
 \emph{Number of elements in the ring buffer.}
 **/
unsigned int fec_rb_cnt = 0;

/*M
 \emph{Ring buffer array.}
 **/
fec_group_t *fec_rb = NULL;

void fec_rb_clear(void) {
    fec_rb_start = 0;
    fec_rb_end = 0;
    fec_rb_cnt = 0;

    int i;
    for (i = 0; i < fec_rb_size; i++)
        fec_group_clear(fec_rb + i);
}

void fec_rb_pop(void) {
    assert(fec_rb != NULL);
    assert(fec_rb_end != fec_rb_start);

    fec_group_destroy(fec_rb + fec_rb_start);
    fec_rb_start = (fec_rb_start + 1) % fec_rb_size;

    fec_rb_cnt--;
}

void fec_rb_destroy(void) {
    if (fec_rb != NULL) {
        while (fec_rb_cnt)
            fec_rb_pop();

        free(fec_rb);
        fec_rb = NULL;
    }
    
    fec_rb_size = 0;
    fec_rb_clear();
}

void fec_rb_init(unsigned int size) {
    fec_rb_destroy();

    fec_rb = malloc(sizeof(fec_group_t) * size);
    assert(fec_rb != NULL);

    int i;
    for (i = 0; i < fec_rb_size; i++)
        fec_group_clear(fec_rb + i);

    fec_rb_size = size;
    fec_rb_clear();
}

unsigned int fec_rb_length(void) {
    assert(fec_rb != NULL);

    if (fec_rb_end >= fec_rb_start)
        return fec_rb_end - fec_rb_start;
    else
        return fec_rb_end + (fec_rb_size - fec_rb_start);
}

int fec_rb_insert_pkt(fec_pkt_t *pkt, int idx) {
    assert(pkt != NULL);
    assert(fec_rb != NULL);

#ifdef DEBUG
    fprintf(stderr, "insert packet at idx %d, gseq %d, pseq %d\n", idx, pkt->hdr.group_seq, pkt->hdr.packet_seq);
#endif

    if (idx < 0) {
        /* try to grow the buffer downwards */
        if ((fec_rb_length() - idx) <= fec_rb_size) {
            fec_rb_start = (fec_rb_start + idx) % fec_rb_size;
            idx = 0;
        }
        else
            /* drop the packet silently */
            return 0;
    }
    else if (idx >= fec_rb_size - 1) {
        /* not enough place left in ring buffer */
        return 0;
    }

    fec_group_t *dst_group = fec_rb + ((idx + fec_rb_start) % fec_rb_size);
    if (dst_group->buf != NULL) {
        assert(dst_group->seq == pkt->hdr.group_seq);
        fec_group_insert_pkt(dst_group, pkt);
    }
    else {
        assert(dst_group->pkts == NULL);
        fec_group_init(dst_group, pkt->hdr.fec_k, pkt->hdr.fec_n, pkt->hdr.group_seq, pkt->hdr.group_tstamp, pkt->hdr.fec_len);
        fec_group_insert_pkt(dst_group, pkt);

        fec_rb_cnt++;
    }

#ifdef DEBUG
    fprintf(stderr, "packet inserted at %d, end: %d\n", (idx + fec_rb_start) % fec_rb_size, fec_rb_end);
#endif

    /* adjust the end pointer */
    if (idx >= fec_rb_length())
        fec_rb_end = ((idx + fec_rb_start + 1) % fec_rb_size);

    assert(fec_rb_start != fec_rb_end);
    /* XXX kann anscheinend auch passieren wenn wraparound. Generell
     muessen hier alle asserts abgefangen werden und mit einer error
     struktur sauber verarbeitet werden. */
    assert(fec_rb[fec_rb_end].buf == NULL);

    return 1;
}

void fec_rb_print(void) {
    assert(fec_rb != NULL);

    fprintf(stderr, "start: %.3u, end: %.3u, len: %.3u\n", fec_rb_start, fec_rb_end, fec_rb_length());
}

void fec_rb_print_rb(void) {
    unsigned int i;
    for (i = fec_rb_start; i != fec_rb_end; i = (i + 1) % fec_rb_size) {
        if (fec_rb[i].buf != NULL)
            fprintf(stderr, "%.3u: seq %.3u\n", i, fec_rb[i].seq);
    }
}

fec_group_t* fec_rb_first(void) {
    assert(fec_rb != NULL);

    if (fec_rb_start == fec_rb_end)
        return NULL;

    /* first should always be the first in buffer */
    return fec_rb + fec_rb_start;
}
