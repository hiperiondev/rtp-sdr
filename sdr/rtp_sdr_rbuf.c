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
// from: https://github.com/embeddedartistry/embedded-resources

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#include "rtp_sdr_rbuf.h"

// This implementation is thread safe for a single producer and single consumer

// The definition of our circular buffer structure is hidden from the user
struct rbuf_s {
    rtp_sdr_sbuf_type_t type;    //
                   iq_t *buffer; //
                 size_t head;    //
                 size_t tail;    //
                 size_t max;     // of the buffer
};

static inline size_t _advance_headtail_value(size_t value, size_t max) {
    if (++value == max) {
        value = 0;
    }

    return value;
}

rbuf_handle_t rtp_sdr_rbuf_init(iq_t *buffer, size_t size, rtp_sdr_sbuf_type_t type) {
    assert(buffer && size > 1);

    rbuf_handle_t cbuf = malloc(sizeof(rbuf_t));
    assert(cbuf);

    cbuf->type = type;
    cbuf->buffer = buffer;
    cbuf->max = size;
    rtp_sdr_rbuf_reset(cbuf);

    assert(rtp_sdr_rbuf_empty(cbuf));

    return cbuf;
}

void rtp_sdr_rbuf_free(rbuf_handle_t me) {
    assert(me);
    free(me);
}

void rtp_sdr_rbuf_reset(rbuf_handle_t me) {
    assert(me);

    me->head = 0;
    me->tail = 0;
}

size_t rtp_sdr_rbuf_size(rbuf_handle_t me) {
    assert(me);

    // We account for the space we can't use for thread safety
    size_t size = me->max - 1;

    if (!rtp_sdr_rbuf_full(me)) {
        if (me->head >= me->tail) {
            size = (me->head - me->tail);
        } else {
            size = (me->max + me->head - me->tail);
        }
    }

    return size;
}

size_t rtp_sdr_rbuf_capacity(rbuf_handle_t me) {
    assert(me);

    // We account for the space we can't use for thread safety
    return me->max - 1;
}

/// For thread safety, do not use put - use try_put.
/// Because this version, which will overwrite the existing contents
/// of the buffer, will involve modifying the tail pointer, which is also
/// modified by get.
void rtp_sdr_rbuf_put(rbuf_handle_t me, iq_t data) {
    assert(me && me->buffer);

    me->buffer[me->head] = data;
    if (rtp_sdr_rbuf_full(me)) {
        // THIS CONDITION IS NOT THREAD SAFE
        me->tail = _advance_headtail_value(me->tail, me->max);
    }

    me->head = _advance_headtail_value(me->head, me->max);
}

int rtp_sdr_rbuf_try_put(rbuf_handle_t me, iq_t data) {
    assert(me);
    assert(me->buffer);

    int r = RTP_SDR_RBUF_ERROR;

    if (!rtp_sdr_rbuf_full(me)) {
        me->buffer[me->head] = data;
        me->head = _advance_headtail_value(me->head, me->max);
        r = RTP_SDR_RBUF_OK;
    }

    return r;
}

int rtp_sdr_rbuf_get(rbuf_handle_t me, iq_t *data) {
    assert(me);
    assert(data);
    assert(me->buffer);

    int r = RTP_SDR_RBUF_ERROR;

    if (!rtp_sdr_rbuf_empty(me)) {
        *data = me->buffer[me->tail];
        me->tail = _advance_headtail_value(me->tail, me->max);
        r = RTP_SDR_RBUF_OK;
    }

    return r;
}

bool rtp_sdr_rbuf_empty(rbuf_handle_t me) {
    assert(me);
    return me->head == me->tail;
}

bool rtp_sdr_rbuf_full(rbuf_handle_t me) {
    // We want to check, not advance, so we don't save the output here
    return _advance_headtail_value(me->head, me->max) == me->tail;
}

int rtp_sdr_rbuf_peek(rbuf_handle_t me, iq_t *data, unsigned int look_ahead_counter) {
    int r = RTP_SDR_RBUF_ERROR;
    size_t pos;

    assert(me && data && me->buffer);

    // We can't look beyond the current buffer size
    if (rtp_sdr_rbuf_empty(me) || look_ahead_counter > rtp_sdr_rbuf_size(me)) {
        return r;
    }

    pos = me->tail;
    for (unsigned int i = 0; i < look_ahead_counter; i++) {
        data[i] = me->buffer[pos];
        pos = _advance_headtail_value(pos, me->max);
    }

    return RTP_SDR_RBUF_OK;
}
