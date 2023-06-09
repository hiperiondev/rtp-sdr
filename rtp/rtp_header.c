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
#include "rtp_header.h"

rtp_header* rtp_header_create(void) {
    rtp_header *header = (rtp_header*) malloc(sizeof(rtp_header));
    if (header)
        memset(header, 0, sizeof(rtp_header));

    return header;
}

void rtp_header_free(rtp_header *header) {
    assert(header != NULL);

    if (header->csrc)
        free(header->csrc);

    if (header->ext_data)
        free(header->ext_data);

    free(header);
}

void rtp_header_init(rtp_header *header, uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts) {
    assert(header != NULL);

    header->version = 2;
    header->pt = (unsigned) (pt & 0x7f);
    header->ssrc = ssrc;
    header->seq = seq;
    header->ts = ts;
}

size_t rtp_header_size(const rtp_header *header) {
    assert(header != NULL);

    size_t size = 12;

    if (header->csrc && header->cc)
        size += 4U * header->cc;

    if (header->x && header->ext_count && header->ext_data)
        size += 4U * (1 + header->ext_count);

    return size;
}

int rtp_header_serialize(const rtp_header *header, uint8_t *buffer, size_t size) {
    assert(header != NULL);
    assert(buffer != NULL);

    const size_t header_size = rtp_header_size(header);
    if (size < header_size)
        return RTP_ERROR;

    // Clear existing data
    memset(buffer, 0, header_size);

    buffer[0] = (2 << 6);
    buffer[1] = (uint8_t) (((header->m & 1) << 7) | (header->pt & 0x7f));

    write_u16(buffer + 2, header->seq);
    write_u32(buffer + 4, header->ts);
    write_u32(buffer + 8, header->ssrc);

    if (header->cc && header->csrc) {
        buffer[0] = (uint8_t) (buffer[0] | (header->cc & 0x7));

        uint8_t *csrc_start = buffer + 12;
        for (uint8_t i = 0; i < header->cc; i++)
            write_u32(csrc_start + (i * 4), header->csrc[i]);
    }

    // Add extension header
    if (header->x && header->ext_count && header->ext_data) {
        buffer[0] |= (1 << 4);

        uint8_t *ext_hdr = buffer + (12 + (4 * header->cc));
        write_u16(ext_hdr, header->ext_id);
        write_u16(ext_hdr + 2, header->ext_count);

        uint8_t *ext_data = ext_hdr + 4;
        for (uint8_t i = 0; i < header->ext_count; i++)
            write_u32(ext_data + (i * 4), header->ext_data[i]);
    }

    return (int) header_size;
}

int rtp_header_parse(rtp_header *header, const uint8_t *buffer, size_t size) {
    assert(header != NULL);
    assert(buffer != NULL);

    // Check initial size
    if (size < 12)
        return RTP_ERROR;

    // Version must be 2
    header->version = (unsigned) ((buffer[0] >> 6) & 0x3);
    if (header->version != 2)
        return RTP_ERROR;

    // Payload type must not be in the range [72-95]
    header->pt = (unsigned) (buffer[1] & 0x7f);
    if (header->pt < 96 && header->pt > 71)
        return RTP_ERROR;

    header->x = (unsigned) ((buffer[0] >> 4) & 0x1);
    header->cc = (unsigned) (buffer[0] & 0x7);
    header->m = (unsigned) ((buffer[1] >> 7) & 0x1);
    header->seq = read_u16(buffer + 2);
    header->ts = read_u32(buffer + 4);
    header->ssrc = read_u32(buffer + 8);

    // Recheck size
    if (size < rtp_header_size(header))
        return RTP_ERROR;

    // Contributing source IDs
    if (header->cc) {
        header->csrc = (uint32_t*) malloc(4 * header->cc);

        const uint8_t *csrc_start = buffer + 12;
        for (uint8_t i = 0; i < header->cc; i++)
            header->csrc[i] = read_u32(csrc_start + (4 * i));
    }

    // Extension header
    if (header->x) {
        const uint8_t *ext_hdr = buffer + (12 + (4 * header->cc));
        header->ext_id = read_u16(ext_hdr);
        header->ext_count = read_u16(ext_hdr + 2);
        header->ext_data = (uint32_t*) malloc(4 * header->ext_count);

        const uint8_t *ext_data = ext_hdr + 4;
        for (uint16_t i = 0; i < header->ext_count; ++i)
            header->ext_data[i] = read_u32(ext_data + (4 * i));
    }

    return RTP_OK;
}

int rtp_header_find_csrc(rtp_header *header, uint32_t csrc) {
    assert(header != NULL);

    for (uint8_t i = 0; i < header->cc; ++i) {
        if (header->csrc[i] == csrc)
            return i;
    }

    return RTP_ERROR;
}

int rtp_header_add_csrc(rtp_header *header, uint32_t csrc) {
    assert(header != NULL);

    if (!header->cc) {
        header->csrc = (uint32_t*) malloc(sizeof(uint32_t));
        header->cc = 1;
    }
    else {
        if (header->cc == 0xff)
            return RTP_ERROR;

        if (rtp_header_find_csrc(header, csrc) != -1)
            return RTP_ERROR;

        header->cc += 1;

        const size_t nmemb = header->cc * sizeof(uint32_t);
        header->csrc = (uint32_t*) realloc(header->csrc, nmemb);
    }

    header->csrc[header->cc - 1] = csrc;

    return RTP_OK;
}

void rtp_header_remove_csrc(rtp_header *header, uint32_t csrc) {
    assert(header != NULL);

    const int index = rtp_header_find_csrc(header, csrc);
    if (index < 0)
        return;

    const size_t size = (unsigned) (header->cc - index) * sizeof(uint32_t);
    if (size)
        memmove(&header->csrc[index], &header->csrc[index + 1], size);

    header->cc -= 1;
    if (header->cc > 0) {
        const size_t nmemb = header->cc * sizeof(uint32_t);
        header->csrc = (uint32_t*) realloc(header->csrc, nmemb);
    }
    else {
        free(header->csrc);
        header->csrc = NULL;
    }
}

int rtp_header_set_ext(rtp_header *header, uint16_t id, const uint32_t *data, uint16_t count) {
    assert(header != NULL);
    assert(data != NULL);

    if (header->ext_data)
        return RTP_ERROR;

    const size_t size = 4 * count;
    header->ext_data = (uint32_t*) malloc(size);
    if (!header->ext_data)
        return -1;

    header->ext_id = id;
    header->ext_count = count;
    memcpy(header->ext_data, data, size);

    return RTP_OK;
}

void rtp_header_clear_ext(rtp_header *header) {
    assert(header != NULL);

    if (header->ext_data) {
        free(header->ext_data);
        header->ext_id = 0;
        header->ext_count = 0;
    }
}
