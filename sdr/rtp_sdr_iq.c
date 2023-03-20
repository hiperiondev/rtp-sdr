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

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "rtp_sdr_iq.h"
#include "rtp_header.h"
#include "rtp_util.h"

#define RTP_PACKET_LENGTH 4096

uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t sample_rate, uint32_t duration, uint32_t host, uint16_t port,
bool use_fec, iq_t *buffer, size_t buffer_size) {
    const int32_t frame_samples = (sample_rate * duration) / 1000;

    switch (type) {
        case IQ_PT8:
            (*session)->frame_size = 2 * frame_samples * sizeof(int8_t);
            break;
        case IQ_PT16:
            (*session)->frame_size = 2 * frame_samples * sizeof(int16_t);
            break;
        case IQ_PT24:
        case IQ_PT32:
            (*session)->frame_size = 2 * frame_samples * sizeof(int32_t);
            break;
        default:
            return RTP_SDR_ERROR;
    }

    (*session)->frame_samples = frame_samples;
    (*session)->use_fec = use_fec;
    (*session)->type = type;
    (*session)->sample_rate = sample_rate;
    (*session)->frequency = frequency;

    (*session)->host = host;
    (*session)->port = port;

    (*session)->iq_buffer = rtp_sdr_rbuf_init(buffer, buffer_size, type);

    (*session)->header = rtp_header_create();
    rtp_header_init((*session)->header, type, rand(), rand(), rand());

    return RTP_SDR_OK;
}

void rcp_iq_deinit(session_iq_t *session) {
    rtp_sdr_rbuf_free((*session)->iq_buffer);
    free(*session);
}

uint8_t rcp_iq_transmit(session_iq_t *session) {
    int32_t n;
    uint8_t data[RTP_PACKET_LENGTH];

    (*session)->header->seq += 1;
    (*session)->header->ts += (*session)->frame_samples;

    int header_size = rtp_header_serialize((*session)->header, data, sizeof(data));
    const uint32_t iq_len = sizeof(RTP_PACKET_LENGTH - header_size) / 2;
    iq_t iq_frame[iq_len];
    if (rtp_sdr_rbuf_peek((*session)->iq_buffer, iq_frame, iq_len) == RTP_SDR_RBUF_ERROR)
        return RTP_SDR_ERROR;

    switch ((*session)->type) {
        case IQ_PT8:
            for (n = 0; n < iq_len; n++) {
                data[header_size++] = iq_frame[n].i.s8;
                data[header_size++] = iq_frame[n].q.s8;
            }
            break;
        case IQ_PT16:
            for (n = 0; n < iq_len / 2; n++) {
                write_u16(data + header_size, iq_frame[n].i.s16);
                header_size += 2;
                write_u16(data + header_size, iq_frame[n].q.s16);
                header_size += 2;
            }
            break;
        case IQ_PT24:
        case IQ_PT32:
            for (n = 0; n < iq_len / 4; n++) {
                write_s24_s32(data + header_size, iq_frame[n].i.s24_s32);
                header_size += 4;
                write_s24_s32(data + header_size, iq_frame[n].q.s24_s32);
                header_size += 4;
            }
            break;
    }

    int error = rtp_socket_send(&(*session)->socket, data, RTP_PACKET_LENGTH);
    if (error < 0) {
        fprintf(stderr, "Failed to send packet: %s\n", strerror(errno));
        return RTP_SDR_ERROR;
    }

    return RTP_SDR_OK;
}

uint8_t rcp_iq_receive(session_iq_t *session, iq_t *data) {

    return RTP_SDR_OK;
}

