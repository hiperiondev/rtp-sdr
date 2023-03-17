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

/**
 * @defgroup app APP packet
 * @brief RTCP APP packet description.
 * @ingroup rtcp
 *
 * @see IETF RFC3550 "Application-Defined RTCP Packet" (§6.7)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P| subtype |     PT=204    |             length            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           SSRC/CSRC                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          name (ASCII)                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                              data                           ...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef LIBRTP_RTCP_APP_H_
#define LIBRTP_RTCP_APP_H_

#include "rtcp_header.h"

/**
 * @brief RTCP APP packet.
 */
typedef struct rtcp_app {
    rtcp_header header; /**< RTCP header. */
    uint32_t ssrc;      /**< Source identifier. */
    uint32_t name;      /**< Packet name (ASCII). */
    size_t app_size;    /**< Size of the application data in bytes. */
    void *app_data;     /**< Application data. */
} rtcp_app;

/**
 * @brief Allocate a new APP packet.
 *
 * @return rtcp_app_packet*
 */
rtcp_app* rtcp_app_create(void);

/**
 * @brief Free an APP packet.
 *
 * @param [out] packet - packet to free.
 */
void rtcp_app_free(rtcp_app *packet);

/**
 * @brief Initialize an APP packet with default values.
 *
 * @param [out] packet - packet to initialize.
 * @param [in] subtype - packet subtype.
 */
void rtcp_app_init(rtcp_app *packet, uint8_t subtype);

/**
 * @brief Returns the APP packet size.
 *
 * @param [in] packet - packet to check.
 * @return packet size in bytes.
 */
size_t rtcp_app_size(const rtcp_app *packet);

/**
 * @brief Write an APP packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_app_serialize(const rtcp_app *packet, uint8_t *buffer, size_t size);

/**
 * @brief Parse an APP packet from a buffer.
 *
 * @param [out] packet - empty packet to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_app_parse(rtcp_app *packet, const uint8_t *buffer, size_t size);

/**
 * @brief Set the application-dependent data.
 *
 * @param [out] packet - packet to set on.
 * @param [in] data - data to set.
 * @param [in] size - data size.
 * @return 0 on success.
 */
int rtcp_app_set_data(rtcp_app *packet, const void *data, size_t size);

/**
 * @brief Clear the application-dependent data.
 *
 * @param [out] packet - packet to clear on.
 */
void rtcp_app_clear_data(rtcp_app *packet);

#endif // LIBRTP_RTCP_APP_H_
