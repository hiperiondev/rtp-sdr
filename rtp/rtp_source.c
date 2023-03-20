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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rtp_source.h"
#include "rtp_util.h"

/**
 * @brief RTP sequence number rollover value.
 * @private
 */
#define LIBRTP_SEQ_MOD (1 << 16)

rtp_source* rtp_source_create(void) {
    rtp_source *s = (rtp_source*) malloc(sizeof(rtp_source));
    if (s)
        memset(s, 0, sizeof(rtp_source));

    return s;
}

void rtp_source_free(rtp_source *s) {
    assert(s != NULL);

    free(s);
}

void rtp_source_init(rtp_source *s, uint32_t id, uint16_t seq) {
    assert(s != NULL);

    rtp_source_reset_seq(s, seq);

    s->id = id;
    s->max_seq = seq - 1;
#if LIBRTP_MIN_SEQUENTIAL > 0
    s->probation = LIBRTP_MIN_SEQUENTIAL;
#endif
}

void rtp_source_reset_seq(rtp_source *s, uint16_t seq) {
    assert(s != NULL);

    s->base_seq = seq;
    s->max_seq = seq;
    s->bad_seq = LIBRTP_SEQ_MOD + 1; // so that 'seq == bad_seq' is false
    s->cycles = 0;
    s->received = 0;
    s->received_prior = 0;
    s->expected_prior = 0;
}

int rtp_source_update_seq(rtp_source *s, uint16_t seq) {
    assert(s != NULL);

#if LIBRTP_MIN_SEQUENTIAL > 0
    // A source is not valid until a number of sequential packets have been
    // received. This can be changed by redefining LIBRTP_MIN_SEQUENTIAL.
    if (s->probation > 0) {
        if (seq == s->max_seq + 1) {
            s->probation--;
            s->max_seq = seq;
            if (s->probation == 0) {
                // Probation done
                rtp_source_reset_seq(s, seq);
                s->received++;
                return 0;
            }
        }
        else {
            s->probation = LIBRTP_MIN_SEQUENTIAL - 1;
            s->max_seq = seq;
        }

        return RTP_ERROR;
    }
#endif

    const uint16_t udelta = seq - s->max_seq;
    if (udelta < LIBRTP_MAX_DROPOUT) {
        // Packets are in order with a permissible gap
        if (seq < s->max_seq)
            s->cycles += LIBRTP_SEQ_MOD; // Sequence number wrapped

        s->max_seq = seq;
    }
    else if (udelta <= LIBRTP_SEQ_MOD - LIBRTP_MAX_MISORDER) {
        // Large jump in sequence number
        if (seq == s->bad_seq) {
            // Two sequential packets were received - assume that the other
            // side restarted without telling us and re-sync.
            // (i.e., pretend this was the first packet).
            rtp_source_reset_seq(s, seq);
        }
        else {
            s->bad_seq = (seq + 1) & (LIBRTP_SEQ_MOD - 1);
            return RTP_ERROR;
        }
    }

    s->received++;
    return RTP_OK;
}

void rtp_source_update_lost(rtp_source *s) {
    assert(s != NULL);

    uint32_t ext_max = s->cycles + s->max_seq;
    int expected = (int) (ext_max - s->base_seq + 1);

    int expected_interval = expected - s->expected_prior;
    int received_interval = s->received - s->received_prior;
    int lost_interval = expected_interval - received_interval;

    s->lost = expected - s->received;
    s->expected_prior = expected;
    s->received_prior = s->received;

    if (expected_interval == 0 || lost_interval <= 0)
        s->fraction = 0;
    else
        s->fraction = (uint8_t) ((lost_interval << 8) / expected_interval);
}

void rtp_source_update_jitter(rtp_source *s, uint32_t ts, uint32_t arrival) {
    assert(s != NULL);

    int transit = (int) (arrival - ts);
    int d = abs(transit - s->transit);

    s->transit = transit;
    s->jitter += (1. / 16.) * ((double) d - s->jitter);
}

void rtp_source_update_lsr(rtp_source *s, ntp_tv tc) {
    assert(s != NULL);

    s->lsr = tc;
}
