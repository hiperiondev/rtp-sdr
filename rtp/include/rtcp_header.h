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

#ifndef RTCP_HEADER_H_
#define RTCP_HEADER_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief RTCP header.
 */
typedef union rtcp_header {
    /** Header format used by most packets. */
    struct {
        unsigned int version :2; /**< Protocol version. */
        unsigned int p :1;       /**< Padding flag. */
        unsigned int count :5;   /**< Varies by packet type */
        unsigned int pt :8;      /**< RTCP packet type */
        uint16_t length;         /**< Length in 32-bit units (less 1) */
    } common;

    /** Header format used by the APP packet. */
    struct {
        unsigned int version :2; /**< Protocol version. */
        unsigned int p :1;       /**< Padding flag. */
        unsigned int subtype :5; /**< App sub-type. */
        unsigned int pt :8;      /**< RTCP packet type. */
        uint16_t length;         /**< Length in 32-bit units (less 1) */
    } app;
} rtcp_header;

/**
 * @brief RTCP packet types.
 */
typedef enum {
    RTCP_SR   = 200,
    RTCP_RR   = 201,
    RTCP_SDES = 202,
    RTCP_BYE  = 203,
    RTCP_APP  = 204
} rtcp_packet_type;

/**
 * @brief Write a RTCP header to a buffer.
 *
 * @param [in] header - header to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_header_serialize(const rtcp_header *header, uint8_t *buffer, size_t size);

/**
 * @brief Parse an RTCP header from a buffer.
 *
 * @param [out] header - empty header to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_header_parse(rtcp_header *header, const uint8_t *buffer, size_t size);

#endif // RTCP_HEADER_H_
