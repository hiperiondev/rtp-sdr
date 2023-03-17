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
 * @defgroup sdes SDES packet
 * @brief RTCP SDES packet description.
 * @ingroup rtcp
 *
 * @see IETF RFC3550 "Source Description RTCP Packet" (§6.5)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|   SC    |     PT=202    |             length            |
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                          SSRC/CSRC (1)                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           SDES items                          |
 *  |                              ...                              |
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                          SSRC/CSRC (2)                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           SDES items                          |
 *  |                              ...                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  :                                                               :
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                          SSRC/CSRC (n)                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           SDES items                          |
 *  |                              ...                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef RTCP_SDES_H_
#define RTCP_SDES_H_

#include <stdint.h>

#include "rtcp_header.h"

/**
 * @brief SDES item types.
 */
typedef enum rtcp_sdes_type {
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8
} rtcp_sdes_type;

/**
 * @brief A description item for an SDES source.
 */
typedef struct rtcp_sdes_item {
    rtcp_sdes_type type; /**< Item type. */
    uint8_t length;      /**< Size of item data in bytes. */
    void *data;          /**< Item data. */
} rtcp_sdes_item;

/**
 * @brief An SDES source description entry.
 */
typedef struct rtcp_sdes_entry {
    uint32_t id;           /**< Source identifier. */
    uint8_t item_count;    /**< Number of items. */
    rtcp_sdes_item *items; /**< Variable length item list. */
} rtcp_sdes_entry;

/**
 * @brief RTCP SDES packet.
 */
typedef struct rtcp_sdes {
    rtcp_header header;    /**< RTCP header. */
    rtcp_sdes_entry *srcs; /**< Variable length source list. */
} rtcp_sdes;

/**
 * @brief Allocate a new SDES packet.
 *
 * @return rtcp_sdes_packet*
 */
rtcp_sdes* rtcp_sdes_create(void);

/**
 * @brief Free an SDES packet.
 *
 * @param [out] packet - packet.
 */
void rtcp_sdes_free(rtcp_sdes *packet);

/**
 * @brief Initialize an SDES packet with default values.
 *
 * @param [out] packet - packet to initialize.
 */
void rtcp_sdes_init(rtcp_sdes *packet);

/**
 * @brief Returns the SDES packet size.
 *
 * @param [in] packet - packet.
 * @return packet size in bytes.
 */
size_t rtcp_sdes_size(const rtcp_sdes *packet);

/**
 * @brief Write an SDES packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_sdes_serialize(const rtcp_sdes *packet, uint8_t *buffer, size_t size);

/**
 * @brief Parse an SDES packet from a buffer.
 *
 * @param [out] packet -packet.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_sdes_parse(rtcp_sdes *packet, const uint8_t *buffer, size_t size);

/**
 * @brief Find a source entry.
 *
 * @param [in] packet - packet to search.
 * @param [in] id - id of the source to find.
 * @return source index or -1 on failure.
 */
int rtcp_sdes_find_entry(struct rtcp_sdes *packet, uint32_t id);

/**
 * @brief Add a source.
 *
 * @param [out] packet - packet to add to.
 * @param [in] id - id of the source to add.
 * @return 0 on success.
 */
int rtcp_sdes_add_entry(struct rtcp_sdes *packet, uint32_t id);

/**
 * @brief Remove a source.
 *
 * @param [out] packet - packet to remove from.
 * @param [in] id - id of the source to remove.
 */
void rtcp_sdes_remove_entry(struct rtcp_sdes *packet, uint32_t id);

/**
 * @brief Get an SDES item.
 *
 * @param [in] packet - packet to get from.
 * @param [in] src - id of the item's source.
 * @param [in] type - item type to get.
 * @param [out] buffer - buffer to write data.
 * @param [in] size - buffer size.
 * @return number of bytes written to buffer.
 */
int rtcp_sdes_get_item(rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type, char *buffer, size_t size);

/**
 * @brief Set an SDES item.
 *
 * @param [out] packet - packet to set on.
 * @param [in] src - id of the item's source.
 * @param [in] type - item type to set.
 * @param [in] data - item data.
 * @return 0 on success.
 */
int rtcp_sdes_set_item(rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type, const char *data);

/**
 * @brief Clear an SDES item.
 *
 * @param [out] packet - packet to clear on.
 * @param [in] src - id of the item's source.
 * @param [in] type - item type to clear.
 */
void rtcp_sdes_clear_item(rtcp_sdes *packet, uint32_t src, rtcp_sdes_type type);

#endif // RTCP_SDES_H_
