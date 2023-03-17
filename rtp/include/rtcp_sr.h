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
 * @defgroup sr SR packet
 * @brief RTCP SR packet description.
 * @ingroup rtcp
 *
 * @see IETF RFC3550 "Sender Report RTCP Packet" (§6.4.1)
 *
 * @verbatim
 *   0               1               2               3
 *   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |V=2|P|   SC    |     PT=200    |             length            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         SSRC of sender                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          NTP timestamp                        |
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          RTP timestamp                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      sender's packet count                    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      sender's octet count                     |
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                      SSRC of first source                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | fraction lost |        cumulative number of packets lost      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           extended highest sequence number received           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       interarrival jitter                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          last SR (LSR)                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                   delay since last SR (DLSR)                  |
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                      SSRC of second source                    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  :                              ...                              :
 *  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  |                   profile-specific extensions                 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

#ifndef RTCP_SR_H_
#define RTCP_SR_H_

#include "rtcp_header.h"
#include "rtcp_report.h"

/**
 * @brief RTCP SR packet.
 */
typedef struct rtcp_sr {
    rtcp_header header;   /**< RTCP header. */
    uint32_t ssrc;        /**< Source identifier. */
    uint32_t ntp_sec;     /**< NTP timestamp MSW. */
    uint32_t ntp_frac;    /**< NTP timestamp LSW. */
    uint32_t rtp_ts;      /**< RTP timestamp. */
    uint32_t pkt_count;   /**< Sender's packet count. */
    uint32_t byte_count;  /**< Sender's byte count. */
    rtcp_report *reports; /**< Reports. */
    size_t ext_size;      /**< Size of the extension data in bytes. */
    void *ext_data;       /**< Extension data. */
} rtcp_sr;

/**
 * @brief Allocate a new SR packet.
 *
 * @return rtcp_sr*
 */
rtcp_sr* rtcp_sr_create(void);

/**
 * @brief Free an SR packet.
 *
 * @param [out] packet - packet to free.
 */
void rtcp_sr_free(rtcp_sr *packet);

/**
 * @brief Initialize an SR packet with default values.
 *
 * @param [out] packet - packet to initialize.
 */
void rtcp_sr_init(rtcp_sr *packet);

/**
 * @brief Returns the SR packet size.
 *
 * @param [in] packet - packet to check.
 * @return packet size in bytes.
 */
size_t rtcp_sr_size(const rtcp_sr *packet);

/**
 * @brief Write an SR packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_sr_serialize(const rtcp_sr *packet, uint8_t *buffer, size_t size);

/**
 * @brief Parse an RR packet from a buffer.
 *
 * @param [out] packet - empty packet to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success
 */
int rtcp_sr_parse(rtcp_sr *packet, const uint8_t *buffer, size_t size);

/**
 * @brief Find a report.
 *
 * @param [in] packet - packet to search.
 * @param [in] ssrc - report source.
 * @return rtcp_report*
 */
rtcp_report* rtcp_sr_find_report(rtcp_sr *packet, uint32_t ssrc);

/**
 * @brief Add a report.
 *
 * @param [out] packet - packet to add to.
 * @param [in] report - report to add.
 * @return 0 on success.
 */
int rtcp_sr_add_report(rtcp_sr *packet, const rtcp_report *report);

/**
 * @brief Remove a report.
 *
 * @param [out] packet - packet to remove from.
 * @param [in] ssrc - source id of the report to remove.
 */
void rtcp_sr_remove_report(rtcp_sr *packet, uint32_t ssrc);

/**
 * @brief Set the extension data.
 *
 * @param [out] packet - packet to set on.
 * @param [in] data - data to set.
 * @param [in] size - data size.
 * @return 0 on success.
 */
int rtcp_sr_set_ext(rtcp_sr *packet, const void *data, size_t size);

/**
 * @brief Clear the extension data.
 *
 * @param [out] packet - packet to clear on.
 */
void rtcp_sr_clear_ext(rtcp_sr *packet);

#endif // RTCP_SR_H_
