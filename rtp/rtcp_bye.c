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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "rtp_util.h"
#include "rtcp_bye.h"

rtcp_bye* rtcp_bye_create(void) {
    rtcp_bye *packet = (rtcp_bye*) malloc(sizeof(rtcp_bye));
    if (packet)
        memset(packet, 0, sizeof(rtcp_bye));

    return packet;
}

void rtcp_bye_free(rtcp_bye *packet) {
    assert(packet != NULL);

    if (!packet->src_ids)
        free(packet->src_ids);

    if (!packet->message)
        free(packet->message);

    free(packet);
}

void rtcp_bye_init(rtcp_bye *packet) {
    assert(packet != NULL);

    packet->header.common.version = 2;
    packet->header.common.pt = RTCP_BYE;
}

size_t rtcp_bye_size(const rtcp_bye *packet) {
    assert(packet != NULL);

    size_t size = 4 + (packet->header.common.count * 4U);
    if (packet->message) {
        size += 1 + strlen(packet->message);
        if (size % 4)
            size += 4 - (size % 4);
    }

    return size;
}

int rtcp_bye_serialize(const rtcp_bye *packet, uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const size_t packet_size = rtcp_bye_size(packet);
    if (size < packet_size)
        return -1;

    if (rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return -1;

    size_t offset = 4;
    for (uint8_t i = 0; i < packet->header.common.count; ++i) {
        write_u32(buffer + offset, packet->src_ids[i]);
        offset += 4;
    }

    if (packet->message) {
        const size_t size = strlen(packet->message);
        if (size > 0xff)
            return -1;

        *(buffer + offset++) = (uint8_t) size;

        memcpy(buffer + offset, packet->message, size);
        offset += size;
    }

    return (int) packet_size;
}

int rtcp_bye_parse(rtcp_bye *packet, const uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    if (size < 4)
        return -1;

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if (pt != RTCP_BYE)
        return -1;

    size_t offset = 4;
    if (packet->header.common.count) {
        // Parse sources
        if (size < 4 + (4U * packet->header.common.count)) {
            free(packet);
            return -1;
        }

        packet->src_ids = (uint32_t*) calloc(packet->header.common.count, sizeof(uint32_t));

        for (uint8_t i = 0; i < packet->header.common.count; ++i) {
            packet->src_ids[i] = read_u32(buffer + offset);
            offset += 4;
        }
    }

    if (size > offset) {
        // Store message
        size_t length = buffer[offset++];
        if (length > size - offset)
            length = size - offset;

        packet->message = (char*) malloc(length + 1);
        memcpy(packet->message, buffer + offset, length);
        packet->message[length + 1] = '\0';
    }

    return 0;
}

int rtcp_bye_find_source(const rtcp_bye *packet, uint32_t src_id) {
    assert(packet != NULL);

    for (uint8_t i = 0; i < packet->header.common.count; ++i) {
        if (packet->src_ids[i] == src_id)
            return i;
    }

    return -1;
}

int rtcp_bye_add_source(rtcp_bye *packet, uint32_t src_id) {
    assert(packet != NULL);

    if (!packet->header.common.count) {
        packet->src_ids = (uint32_t*) malloc(sizeof(uint32_t));
        packet->header.common.count = 1;
    }
    else {
        if (packet->header.common.count == 0xff)
            return -1;

        if (rtcp_bye_find_source(packet, src_id) != -1)
            return -1;

        packet->header.common.count += 1;

        const size_t nmemb = packet->header.common.count * sizeof(uint32_t);
        packet->src_ids = (uint32_t*) realloc(packet->src_ids, nmemb);
    }

    packet->src_ids[packet->header.common.count - 1] = src_id;

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_bye_size(packet) / 4) - 1);

    return 0;
}

void rtcp_bye_remove_source(rtcp_bye *packet, uint32_t src_id) {
    assert(packet != NULL);

    const int index = rtcp_bye_find_source(packet, src_id);
    if (index < 0)
        return;

    const size_t size = (unsigned) (packet->header.common.count - index) * sizeof(uint32_t);

    if (size)
        memmove(&packet->src_ids[index], &packet->src_ids[index + 1], size);

    packet->header.common.count -= 1;
    if (packet->header.common.count > 0) {
        const size_t nmemb = packet->header.common.count * sizeof(uint32_t);
        packet->src_ids = (uint32_t*) realloc(packet->src_ids, nmemb);
    }
    else {
        free(packet->src_ids);
        packet->src_ids = NULL;
    }

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_bye_size(packet) / 4) - 1);
}

int rtcp_bye_set_message(rtcp_bye *packet, const char *message) {
    assert(packet != NULL);
    assert(message != NULL);

    if (packet->message)
        return -1;

    const size_t size = strlen(message);
    packet->message = (char*) malloc(size + 1);
    if (!packet->message)
        return -1;

    sprintf(packet->message, "%s", message);

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_bye_size(packet) / 4) - 1);

    return 0;
}

void rtcp_bye_clear_message(rtcp_bye *packet) {
    assert(packet != NULL);

    free(packet->message);
    packet->message = NULL;

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_bye_size(packet) / 4) - 1);
}
