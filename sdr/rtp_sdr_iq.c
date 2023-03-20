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
#include <unistd.h>
#include <sys/time.h>

#include "rtp_sdr_iq.h"
#include "rtp_header.h"
#include "rtp_util.h"

#define RTP_PACKET_LENGTH 4096

static void _isleep(int usec) {
    int diff;
    struct timeval start, now;

    gettimeofday(&start, NULL);
    diff = 0;
    while (diff < usec) {
        /* If enough time to sleep, otherwise, busywait */
        if (usec - diff > 200) {
            usleep(usec - 20);
        }
        gettimeofday(&now, NULL);
        diff = now.tv_sec - start.tv_sec;
        diff *= 1000000;
        diff += now.tv_usec - start.tv_usec;
    }
}

uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t tx_sample_rate, sample_rate_t rx_sample_rate, uint32_t duration,
        uint32_t host, uint16_t port, bool use_fec, iq_t *tx_buffer, iq_t *rx_buffer, size_t buffer_size, uint8_t tx_qty, uint8_t rx_qty) {

    (*session)->tx_enabled = false;
    (*session)->tx_frame_samples = (tx_sample_rate * duration) / 1000;
    (*session)->rx_frame_samples = (rx_sample_rate * duration) / 1000;
    (*session)->use_fec = use_fec;
    (*session)->type = type;
    (*session)->rx_sample_rate = rx_sample_rate;
    (*session)->rx_sample_rate = rx_sample_rate;
    (*session)->tx_frequency = malloc(sizeof(double) * tx_qty);
    (*session)->rx_frequency = malloc(sizeof(double) * rx_qty);

    (*session)->host = host;
    (*session)->port = port;

    (*session)->tx_iq_buffer = rtp_sdr_rbuf_init(tx_buffer, buffer_size, type);
    (*session)->rx_iq_buffer = rtp_sdr_rbuf_init(rx_buffer, buffer_size, type);

    (*session)->rx_header = NULL;
    (*session)->tx_header = rtp_header_create();
    rtp_header_init((*session)->tx_header, type, rand(), rand(), rand());

    return RTP_SDR_OK;
}

void rcp_iq_deinit(session_iq_t *session) {
    rtp_sdr_rbuf_free((*session)->tx_iq_buffer);
    rtp_sdr_rbuf_free((*session)->rx_iq_buffer);

    rtp_header_free((*session)->tx_header);

    free(*session);
}

uint8_t rcp_iq_transmit(session_iq_t *session) {
    int32_t n;
    uint8_t data[RTP_PACKET_LENGTH];
    struct timeval start, now;
    int header_size;
    uint32_t iq_len;
    int packet_len = 0;

    gettimeofday(&start, NULL);

    (*session)->tx_header->seq += 1;
    (*session)->tx_header->ts += (*session)->tx_frame_samples;

    header_size = rtp_header_serialize((*session)->tx_header, data, sizeof(data));
    iq_len = sizeof(RTP_PACKET_LENGTH - header_size) / 2;
    iq_t iq_frame[iq_len];
    if (rtp_sdr_rbuf_peek((*session)->tx_iq_buffer, iq_frame, iq_len) == RTP_SDR_RBUF_ERROR)
        return RTP_SDR_ERROR;

    switch ((*session)->type) {
        case IQ_PT8:
            packet_len = iq_len;
            for (n = 0; n < packet_len; n++) {
                data[header_size++] = iq_frame[n].i.s8;
                data[header_size++] = iq_frame[n].q.s8;
            }
            break;
        case IQ_PT16:
            packet_len = iq_len / 2;
            for (n = 0; n < packet_len; n++) {
                write_u16(data + header_size, iq_frame[n].i.s16);
                header_size += 2;
                write_u16(data + header_size, iq_frame[n].q.s16);
                header_size += 2;
            }
            break;
        case IQ_PT24:
        case IQ_PT32:
            packet_len = iq_len / 4;
            for (n = 0; n < packet_len; n++) {
                write_s24_s32(data + header_size, iq_frame[n].i.s24_s32);
                header_size += 4;
                write_s24_s32(data + header_size, iq_frame[n].q.s24_s32);
                header_size += 4;
            }
            break;
    }

    int error = rtp_socket_send(&((*session)->socket), data, RTP_PACKET_LENGTH);
    if (error < 0) {
        fprintf(stderr, "Failed to send packet: %s\n", strerror(errno));
        return RTP_SDR_ERROR;
    }

    // pause by rate
    gettimeofday(&now, NULL);
    double rate_now;
    long duration_now;
    duration_now = (now.tv_sec - start.tv_sec);
    duration_now *= 1000000;
    duration_now += now.tv_usec - start.tv_usec;
    rate_now = packet_len * 8 * 1000;
    rate_now = rate_now / duration_now;
    if (rate_now > (*session)->tx_sample_rate) {
        long int_delay = packet_len * 8 * 1000;
        int_delay = int_delay / (*session)->tx_sample_rate;
        int_delay = int_delay - duration_now;

        if ((int_delay <= 0) || (int_delay > 10000000))
            fprintf(stderr, "!!! BIG delay !!!  %ld\n", int_delay);
        if (int_delay > 0)
            _isleep(int_delay);
    }

    return RTP_SDR_OK;
}

uint8_t rcp_iq_receive(session_iq_t *session) {
    uint8_t data[RTP_PACKET_LENGTH];
    int n, pos = 0;
    iq_t iq_data;

    int packet_len = rtp_socket_recv(&((*session)->socket), data, sizeof(data));
    if (packet_len < 0) {
        fprintf(stderr, "Failed to receive packet: %s\n", strerror(errno));
        return RTP_SDR_ERROR;
    }

    (*session)->rx_header = rtp_header_create();
    if (rtp_header_parse((*session)->rx_header, data, packet_len) < 0) {
        fprintf(stderr, "Bad packet - dropping\n");
        return RTP_SDR_ERROR;
    }

    int header_size = rtp_header_size((*session)->rx_header);
    uint8_t *payload = data + header_size;
    int payload_size = packet_len - header_size;

    switch ((*session)->type) {
        case IQ_PT8:
            for (n = 0; n < payload_size; n++) {
                iq_data.i.s8 = payload[pos++];
                iq_data.i.s8 = payload[pos++];
                rtp_sdr_rbuf_put((*session)->rx_iq_buffer, iq_data);
            }
            break;
        case IQ_PT16:
            for (n = 0; n < payload_size; n++) {
                iq_data.i.s16 = read_u16(payload + pos);
                pos += 2;
                iq_data.q.s16 = read_u16(payload + pos);
                pos += 2;
                rtp_sdr_rbuf_put((*session)->rx_iq_buffer, iq_data);
            }
            break;
        case IQ_PT24:
            for (n = 0; n < payload_size; n++) {
                iq_data.i.s24_s32 = read_s24(payload + pos);
                pos += 4;
                iq_data.q.s24_s32 = read_s24(payload + pos);
                pos += 4;
                rtp_sdr_rbuf_put((*session)->rx_iq_buffer, iq_data);
            }
            break;
        case IQ_PT32:
            for (n = 0; n < payload_size; n++) {
                iq_data.i.s24_s32 = read_u32(payload + pos);
                pos += 4;
                iq_data.q.s24_s32 = read_u32(payload + pos);
                pos += 4;
                rtp_sdr_rbuf_put((*session)->rx_iq_buffer, iq_data);
            }
            break;
        default:
            return RTP_SDR_ERROR;
    }

    rtp_header_free((*session)->rx_header);
    (*session)->rx_header = NULL;

    return RTP_SDR_OK;
}

void* rcp_iq_transmit_handler(void *arg) {
    session_iq_t *session = (session_iq_t*) arg;

    while (1) {
        while ((*session)->tx_enabled) {
            if(rcp_iq_receive(session) == RTP_SDR_ERROR)
                return NULL;
        }

        usleep(1000);
    }

    return NULL;
}

void* rcp_iq_receive_handler(void *arg) {

    return NULL;
}
