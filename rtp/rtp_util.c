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

#include "rtp_util.h"

void write_u24(uint8_t *buffer, uint32_t value) {
    buffer[0] = (value >> 16) & 0xff;
    buffer[1] = (value >> 8) & 0xff;
    buffer[2] = (value >> 0) & 0xff;
}

void write_s24(uint8_t *buffer, int32_t value) {
    uint32_t u;
    memcpy(&u, &value, 4);
    write_u24(buffer, u);
}

void write_u32(uint8_t *buffer, uint32_t value) {
    buffer[0] = (uint8_t) ((value >> 24) & 0xff);
    buffer[1] = (uint8_t) ((value >> 16) & 0xff);
    buffer[2] = (uint8_t) ((value >> 8) & 0xff);
    buffer[3] = (uint8_t) ((value >> 0) & 0xff);
}

uint32_t read_u24(const uint8_t *buffer) {
    return ((uint32_t) buffer[0] << 16) | ((uint32_t) buffer[1] << 8) | ((uint32_t) buffer[2] << 0);
}

int32_t read_s24(const uint8_t *buffer) {
    // Read in the two's complement representation
    const uint32_t u = read_u24(buffer);
    if (u < 0x800000)
        return (int32_t) u;

    return -1 * (int32_t) (((~u) & 0xffffff) + 1);
}

uint32_t read_u32(const uint8_t *buffer) {
    return ((uint32_t) buffer[0] << 24) | ((uint32_t) buffer[1] << 16) | ((uint32_t) buffer[2] << 8) | ((uint32_t) buffer[3] << 0);
}

void write_u16(uint8_t *buffer, uint16_t value) {
    buffer[0] = (uint8_t) ((value >> 8) & 0xff);
    buffer[1] = (uint8_t) (value & 0xff);
}

uint16_t read_u16(const uint8_t *buffer) {
    return (uint16_t) (((uint16_t) buffer[0] << 8) | buffer[1]);
}
