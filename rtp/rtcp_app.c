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

#include "rtp_util.h"
#include "rtcp_app.h"

rtcp_app* rtcp_app_create(void) {
    rtcp_app *packet = (rtcp_app*) malloc(sizeof(rtcp_app));
    if (packet)
        memset(packet, 0, sizeof(rtcp_app));

    return packet;
}

void rtcp_app_free(rtcp_app *packet) {
    assert(packet != NULL);

    if (packet->app_data)
        free(packet->app_data);

    free(packet);
}

void rtcp_app_init(rtcp_app *packet, uint8_t subtype) {
    assert(packet != NULL);

    packet->header.app.version = 2;
    packet->header.app.subtype = (unsigned) (subtype & 0x1f);
    packet->header.app.pt = RTCP_APP;
    packet->header.app.length = 2;
}

size_t rtcp_app_size(const rtcp_app *packet) {
    assert(packet != NULL);

    size_t size = 12;
    if (packet->app_data && packet->app_size) {
        size += packet->app_size;
        if (size % 4)
            size += 4 - (size % 4);
    }

    return size;
}

int rtcp_app_serialize(const rtcp_app *packet, uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const size_t packet_size = rtcp_app_size(packet);
    if (size < packet_size)
        return -1;

    memset(buffer, 0, packet_size);
    if (rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return -1;

    write_u32(buffer + 4, packet->ssrc);
    write_u32(buffer + 8, packet->name);

    if (packet->app_data)
        memcpy(buffer + 12, packet->app_data, packet->app_size);

    return (int) packet_size;
}

int rtcp_app_parse(rtcp_app *packet, const uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if (pt != RTCP_APP)
        return -1;

    packet->ssrc = read_u32(buffer + 4);
    packet->name = read_u32(buffer + 8);

    const size_t length = (packet->header.common.length + 1) * 4U;
    if (length > 12) {
        packet->app_size = length - 12;
        packet->app_data = malloc(packet->app_size);
        memcpy(packet->app_data, buffer + 12, packet->app_size);
    }

    return 0;
}

int rtcp_app_set_data(rtcp_app *packet, const void *data, size_t size) {
    assert(packet != NULL);
    assert(data != NULL);

    if (packet->app_data)
        return -1;

    packet->app_data = malloc(size);
    if (!packet->app_data)
        return -1;

    packet->app_size = size;
    memcpy(packet->app_data, data, size);

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_app_size(packet) / 4) - 1);

    return 0;
}

void rtcp_app_clear_data(rtcp_app *packet) {
    assert(packet != NULL);

    if (packet->app_data) {
        free(packet->app_data);
        packet->app_data = NULL;
        packet->app_size = 0;
    }

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_app_size(packet) / 4) - 1);
}
