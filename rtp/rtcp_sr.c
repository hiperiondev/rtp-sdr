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
#include "rtcp_util.h"
#include "rtcp_sr.h"

rtcp_sr* rtcp_sr_create(void) {
    rtcp_sr *packet = (rtcp_sr*) malloc(sizeof(rtcp_sr));
    if (packet)
        memset(packet, 0, sizeof(rtcp_sr));

    return packet;
}

void rtcp_sr_free(rtcp_sr *packet) {
    assert(packet != NULL);

    if (packet->ext_data)
        free(packet->ext_data);

    free(packet);
}

void rtcp_sr_init(rtcp_sr *packet) {
    assert(packet != NULL);

    packet->header.common.version = 2;
    packet->header.common.pt = RTCP_SR;
    packet->header.common.length = 6;
}

size_t rtcp_sr_size(const rtcp_sr *packet) {
    assert(packet != NULL);

    size_t size = 28 + (packet->header.common.count * sizeof(rtcp_report));
    if (packet->ext_data && packet->ext_size) {
        size += packet->ext_size;
        if (size % 4)
            size += 4 - (size % 4);
    }

    return size;
}

int rtcp_sr_serialize(const rtcp_sr *packet, uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const size_t packet_size = rtcp_sr_size(packet);
    if (size < packet_size)
        return RTCP_ERROR;

    if (rtcp_header_serialize(&packet->header, buffer, size) < 0)
        return RTCP_ERROR;

    write_u32(buffer + 4, packet->ssrc);
    write_u32(buffer + 8, packet->ntp_sec);
    write_u32(buffer + 12, packet->ntp_frac);
    write_u32(buffer + 16, packet->rtp_ts);
    write_u32(buffer + 20, packet->pkt_count);
    write_u32(buffer + 24, packet->byte_count);

    size_t offset = 28;
    for (uint8_t i = 0; i < packet->header.common.count; ++i) {
        const int result = rtcp_report_serialize(&packet->reports[i], buffer + offset, size - offset);

        if (result < 0)
            return RTCP_ERROR;

        offset += (unsigned) result;
    }

    if (packet->ext_data)
        memcpy(buffer + offset, packet->ext_data, packet->ext_size);

    return (int) packet_size;
}

int rtcp_sr_parse(rtcp_sr *packet, const uint8_t *buffer, size_t size) {
    assert(packet != NULL);
    assert(buffer != NULL);

    const int pt = rtcp_header_parse(&packet->header, buffer, size);
    if (pt != RTCP_SR)
        return RTCP_ERROR;

    packet->ssrc = read_u32(buffer + 4);
    packet->ntp_sec = read_u32(buffer + 8);
    packet->ntp_frac = read_u32(buffer + 12);
    packet->rtp_ts = read_u32(buffer + 16);
    packet->pkt_count = read_u32(buffer + 20);
    packet->byte_count = read_u32(buffer + 24);

    size_t offset = 28;
    if (packet->header.common.count) {
        packet->reports = (rtcp_report*) calloc(packet->header.common.count, sizeof(rtcp_report));

        for (uint8_t i = 0; i < packet->header.common.count; ++i) {
            rtcp_report *report = &packet->reports[i];
            if (rtcp_report_parse(report, buffer + offset, size - offset) < 0)
                return RTCP_ERROR;

            offset += sizeof(rtcp_report);
        }
    }

    const size_t length = (packet->header.common.length + 1) * 4U;
    packet->ext_size = length - offset;

    if (packet->ext_size) {
        packet->ext_data = malloc(packet->ext_size);
        memcpy(packet->ext_data, buffer + offset, packet->ext_size);
    }

    return RTCP_OK;
}

rtcp_report* rtcp_sr_find_report(rtcp_sr *packet, uint32_t src_id) {
    assert(packet != NULL);

    for (uint8_t i = 0; i < packet->header.common.count; ++i) {
        rtcp_report *report = &packet->reports[i];
        if (report->ssrc == src_id)
            return report;
    }

    return NULL;
}

int rtcp_sr_add_report(rtcp_sr *packet, const rtcp_report *report) {
    assert(packet != NULL);
    assert(report != NULL);

    if (!packet->header.common.count) {
        packet->reports = (rtcp_report*) malloc(sizeof(rtcp_report));
        packet->header.common.count = 1;
    }
    else {
        if (packet->header.common.count == 0xff)
            return RTCP_ERROR;

        if (rtcp_sr_find_report(packet, report->ssrc))
            return RTCP_ERROR;

        packet->header.common.count += 1;

        const size_t nmemb = packet->header.common.count * sizeof(rtcp_report);
        packet->reports = (rtcp_report*) realloc(packet->reports, nmemb);
    }

    rtcp_report *dest = &packet->reports[packet->header.common.count - 1];
    memcpy(dest, report, sizeof(rtcp_report));

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_sr_size(packet) / 4) - 1);

    return RTCP_OK;
}

void rtcp_sr_remove_report(rtcp_sr *packet, uint32_t src_id) {
    assert(packet != NULL);

    rtcp_report *report = rtcp_sr_find_report(packet, src_id);
    if (!report)
        return;

    const ptrdiff_t offset = report - packet->reports;
    assert(offset >= 0);

    const size_t index = (size_t) offset / sizeof(rtcp_report);
    const size_t size = (packet->header.common.count - index) * sizeof(uint32_t);
    if (size)
        memmove(report, report + 1, size);

    packet->header.common.count -= 1;
    if (packet->header.common.count > 0) {
        const size_t nmemb = packet->header.common.count * sizeof(rtcp_report);
        packet->reports = (rtcp_report*) realloc(packet->reports, nmemb);
    }
    else {
        free(packet->reports);
        packet->reports = NULL;
    }

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_sr_size(packet) / 4) - 1);
}

int rtcp_sr_set_ext(rtcp_sr *packet, const void *data, size_t size) {
    assert(packet != NULL);
    assert(data != NULL);

    if ((size % 4) != 0)
        return -1;

    if (packet->ext_data)
        return RTCP_ERROR;

    packet->ext_data = malloc(size);
    if (packet->ext_data == NULL)
        return RTCP_ERROR;

    packet->ext_size = size;
    memcpy(packet->ext_data, data, size);

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_sr_size(packet) / 4) - 1);

    return RTCP_OK;
}

void rtcp_sr_clear_ext(rtcp_sr *packet) {
    assert(packet != NULL);

    if (packet->ext_data) {
        free(packet->ext_data);
        packet->ext_data = NULL;
        packet->ext_size = 0;
    }

    // Update header length
    packet->header.common.length = (uint16_t) ((rtcp_sr_size(packet) / 4) - 1);
}
