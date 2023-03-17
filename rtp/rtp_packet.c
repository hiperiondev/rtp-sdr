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

#include "rtp_packet.h"

rtp_packet* rtp_packet_create(void) {
    rtp_packet *packet = (rtp_packet*) malloc(sizeof(rtp_packet));
    if (packet) {
        memset(packet, 0, sizeof(rtp_packet));
        packet->header = rtp_header_create();

        if (!packet->header) {
            free(packet);
            packet = NULL;
        }
    }

    return packet;
}

void rtp_packet_free(rtp_packet *packet) {
    assert(packet != NULL);

    if (packet->header)
        rtp_header_free(packet->header);

    if (packet->payload_data)
        free(packet->payload_data);

    free(packet);
}

void rtp_packet_init(rtp_packet *packet, uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts) {
    assert(packet != NULL);

    rtp_header_init(packet->header, pt, ssrc, seq, ts);
}

size_t rtp_packet_size(const rtp_packet *packet) {
    assert(packet != NULL);

    return rtp_header_size(packet->header) + packet->payload_size;
}

int rtp_packet_serialize(const rtp_packet *packet, uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const size_t packet_size = rtp_packet_size(packet);
    if (size < packet_size)
        return -1;

    // Add header
    int offset = rtp_header_serialize(packet->header, buffer, size);
    if (offset < 0)
        return offset;

    // Add payload
    memcpy(buffer + offset, packet->payload_data, packet->payload_size);

    return (int) packet_size;
}

int rtp_packet_parse(rtp_packet *packet, const uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    if (rtp_header_parse(packet->header, buffer, size) < 0)
        return -1;

    const size_t header_size = rtp_header_size(packet->header);
    rtp_packet_set_payload(packet, buffer + header_size, size - header_size);

    return 0;
}

int rtp_packet_set_payload(rtp_packet *packet, const void *data, size_t size) {
    assert(packet != NULL);
    assert(data != NULL);

    if (packet->payload_data)
        return -1;

    packet->payload_data = malloc(size);
    if (!packet->payload_data)
        return -1;

    packet->payload_size = size;
    memcpy(packet->payload_data, data, size);

    return 0;
}

void rtp_packet_clear_payload(rtp_packet *packet) {
    assert(packet != NULL);

    if (packet->payload_data) {
        free(packet->payload_data);
        packet->payload_data = NULL;
        packet->payload_size = 0;
    }
}
