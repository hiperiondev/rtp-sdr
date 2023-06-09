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
 * @defgroup rtp RTP
 * @brief Real-time transport protocol.
 * @see IETF RFC3550 "RTP Fixed Header Fields" (§5.1)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           timestamp                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           synchronization source (SSRC) identifier            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |            contributing source (CSRC) identifiers             |
 *  |                              ...                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef RTP_HEADER_H_
#define RTP_HEADER_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief RTP packet.
 */
typedef struct rtp_header {
    // Required
    unsigned int version :2; /**< protocol version */
    unsigned int p :1;       /**< padding flag */
    unsigned int x :1;       /**< Header extension flag */
    unsigned int cc :4;      /**< CSRC count */
    unsigned int m :1;       /**< Marker bit */
    unsigned int pt :7;      /**< Payload type */
    unsigned int seq :16;    /**< Sequence number */
    uint32_t ts;             /**< Timestamp */
    uint32_t ssrc;           /**< Synchronization source */

    // Optional
    uint32_t *csrc;          /**< List of contributing sources. */

    uint16_t ext_id;         /**< Extension ID. */
    uint16_t ext_count;      /**< Number of extension entries. */
    uint32_t *ext_data;      /**< Extension data. */
} rtp_header;

/**
 * @brief Allocate a new RTP header.
 *
 * @return header.
 */
rtp_header* rtp_header_create(void);

/**
 * @brief Free an RTP header.
 *
 * @param [out] header - header to free.
 */
void rtp_header_free(rtp_header *header);

/**
 * @brief Initialize an RTP header.
 *
 * @param [out] header - header to initialize.
 * @param [in] pt - payload type.
 * @param [in] ssrc - synchronization source identifier.
 * @param [in] seq - sequence number.
 * @param [in] ts - packet timestamp.
 */
void rtp_header_init(rtp_header *header, uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts);

/**
 * @brief Returns the RTP header size.
 *
 * @param [in] header - header to check.
 * @return header size in bytes.
 */
size_t rtp_header_size(const rtp_header *header);

/**
 * @brief Write an RTP header to a buffer.
 *
 * @param [in] header - header to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtp_header_serialize(const rtp_header *header, uint8_t *buffer, size_t size);

/**
 * @brief Fill a RTP header from a buffer.
 *
 * @param [out] header - empty header to fill.
 * @param [in] buffer - buffer to read from.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtp_header_parse(rtp_header *header, const uint8_t *buffer, size_t size);

/**
 * @brief Return the index of a csrc.
 *
 * @param [in] header - header to search.
 * @param [in] csrc - csrc to find.
 * @return csrc index or -1 on failure.
 */
int rtp_header_find_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Add a contributing source id.
 *
 * @param [out] header - header to add to.
 * @param [in] csrc - id of the csrc to add.
 * @return 0 on success.
 */
int rtp_header_add_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Remove a contributing source id.
 *
 * @param [out] header - header to remove from.
 * @param [in] csrc - id of the csrc to remove.
 */
void rtp_header_remove_csrc(rtp_header *header, uint32_t csrc);

/**
 * @brief Set the header extension.
 *
 * @param [out] header - header to set on.
 * @param [in] id - extension id.
 * @param [in] data - extension data.
 * @param [in] count - the number of entries in data.
 * @return 0 on success.
 */
int rtp_header_set_ext(rtp_header *header, uint16_t id, const uint32_t *data, uint16_t count);

/**
 * @brief Clear the header extension.
 *
 * @param [out] header - header to clear on.
 */
void rtp_header_clear_ext(rtp_header *header);

#endif // RTP_HEADER_H_
