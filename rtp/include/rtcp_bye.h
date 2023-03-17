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

/**
 * @defgroup bye BYE packet
 * @brief RTCP BYE packet description.
 * @ingroup rtcp
 *
 * @see IETF RFC3550 "Goodbye RTCP Packet" (§6.6)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|   SC    |     PT=203    |             length            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           SSRC/CSRC                           |
 *  |                              ...                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     length    |             reason for leaving              ...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef RTCP_BYE_H_
#define RTCP_BYE_H_

#include "rtcp_header.h"

/**
 * @brief RTCP BYE packet.
 */
typedef struct rtcp_bye {
    rtcp_header header; /**< RTCP header. */
    uint32_t *src_ids; /**< Source identifiers. */
    char *message; /**< Reason for leaving. */
} rtcp_bye;

/**
 * @brief Allocate a new BYE packet.
 *
 * @return rtcp_bye_packet*
 */
rtcp_bye* rtcp_bye_create(void);

/**
 * @brief Free a BYE packet.
 *
 * @param [out] packet - packet to free.
 */
void rtcp_bye_free(rtcp_bye *packet);

/**
 * @brief Initialize a BYE packet with default values.
 *
 * @param [out] packet - packet to initialize.
 */
void rtcp_bye_init(rtcp_bye *packet);

/**
 * @brief Returns the BYE packet size.
 *
 * @param [in] packet - packet to check.
 * @return packet size in bytes.
 */
size_t rtcp_bye_size(const rtcp_bye *packet);

/**
 * @brief Write a BYE packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_bye_serialize(const rtcp_bye *packet, uint8_t *buffer, size_t size);

/**
 * @brief Parse a BYE packet from a buffer.
 *
 * @param [out] packet - empty packet to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_bye_parse(rtcp_bye *packet, const uint8_t *buffer, size_t size);

/**
 * @brief Return the index of a source.
 *
 * @param [in] packet - packet to search.
 * @param [in] src_id - source to find.
 * @return source index or -1 on failure.
 */
int rtcp_bye_find_source(const rtcp_bye *packet, uint32_t src_id);

/**
 * @brief Add a source to the packet.
 *
 * @param [out] packet - packet to add to.
 * @param [in] src_id - source to add.
 * @return 0 on success.
 */
int rtcp_bye_add_source(rtcp_bye *packet, uint32_t src_id);

/**
 * @brief Remove a source from the packet.
 *
 * @param [out] packet - packet to remove from.
 * @param [in] src_id - source to remove.
 */
void rtcp_bye_remove_source(rtcp_bye *packet, uint32_t src_id);

/**
 * @brief Set the BYE message.
 *
 * @param [out] packet - packet to set on.
 * @param [in] message - message to set.
 * @return 0 on success.
 */
int rtcp_bye_set_message(rtcp_bye *packet, const char *message);

/**
 * @brief Clear the BYE message.
 *
 * @param [out] packet - packet.
 */
void rtcp_bye_clear_message(rtcp_bye *packet);


#endif // RTCP_BYE_H_
