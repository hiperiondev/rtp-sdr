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

#include <stdint.h>
#include <stdlib.h>

#include "rtp_iq.h"

uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t sample_rate, uint32_t duration, uint32_t host, uint16_t port) {
    const int32_t frame_samples = (sample_rate * duration) / 1000;
    switch (type) {
        case IQ_PT8:
            (*session)->frame._8 = (int8_t*) malloc(2 * frame_samples * sizeof(int8_t));
            break;
        case IQ_PT16:
            (*session)->frame._16 = (int16_t*) malloc(2 * frame_samples * sizeof(int16_t));
            break;
        case IQ_PT24:
            (*session)->frame._24 = (int32_t*) malloc(2 * frame_samples * sizeof(int32_t));
            break;
        default:
            return -1;
    }

    (*session)->type = type;
    (*session)->sample_rate = sample_rate;
    (*session)->frequency = frequency;

    (*session)->host = host;
    (*session)->port = port;

    (*session)->header = rtp_header_create();
    rtp_header_init((*session)->header, type, rand(), rand(), rand());

    return 0;
}

void rcp_iq_deinit(session_iq_t *session) {
    switch ((*session)->type) {
        case IQ_PT8:
            free((*session)->frame._8);
            break;
        case IQ_PT16:
            free((*session)->frame._16);
            break;
        case IQ_PT24:
            free((*session)->frame._24);
            break;
    }

    free(*session);
}

