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

#include <string.h>
#include <assert.h>

#include "rtp_util.h"
#include "rtcp_header.h"

int rtcp_header_serialize(const rtcp_header *header, uint8_t *buffer, size_t size) {
    assert(header != NULL);
    assert(buffer != NULL);

    if (size < 4)
        return -1;

    buffer[0] = (uint8_t) ((header->common.version << 6) | (header->common.p << 5) | (header->common.count));

    buffer[1] = header->common.pt;
    write_u16(buffer + 2, header->common.length);

    return 4;
}

int rtcp_header_parse(rtcp_header *header, const uint8_t *buffer, size_t size) {
    assert(header != NULL);
    assert(buffer != NULL);

    if (size < 4)
        return -1;

    header->common.version = (unsigned) ((buffer[0] >> 6) & 0x3);
    header->common.p = (unsigned) ((buffer[0] >> 5) & 0x1);
    header->common.count = (unsigned) (buffer[0] & 0x1f);
    header->common.pt = buffer[1];
    header->common.length = read_u16(buffer + 2);

    if (header->common.version != 2)
        return -1;

    return header->common.pt;
}
