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

#ifndef RTP_PACKET_H_
#define RTP_PACKET_H_

#include "rtp_header.h"

/**
 * @brief RTP packet.
 */
typedef struct rtp_packet {
    rtp_header *header;  /**< RTP header. */
    size_t payload_size; /**< Size of the payload data in bytes. */
    void *payload_data;  /**< Payload data. */
} rtp_packet;

/**
 * @brief Allocate a new RTP packet.
 *
 * @return packet.
 */
rtp_packet* rtp_packet_create(void);

/**
 * @brief Free a RTP packet.
 *
 * @param [out] packet - packet to free.
 */
void rtp_packet_free(rtp_packet *packet);

/**
 * @brief Initialize an RTP packet with default values.
 *
 * @param [out] packet - packet to initialize.
 * @param [in] pt - payload type.
 * @param [in] ssrc - synchronization source identifier.
 * @param [in] seq - sequence number.
 * @param [in] ts - packet timestamp.
 */
void rtp_packet_init(rtp_packet *packet, uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts);

/**
 * @brief Returns the RTP packet size.
 *
 * @param [in] packet - packet to check.
 * @return packet size in bytes.
 */
size_t rtp_packet_size(const rtp_packet *packet);

/**
 * @brief Write an RTP packet to a buffer.
 *
 * @param [in] packet - packet to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtp_packet_serialize(const rtp_packet *packet, uint8_t *buffer, size_t size);

/**
 * @brief Fill an RTP packet from a buffer.
 *
 * This takes a copy of the payload. For a non-copy approach you can use the
 * rtp header api directly.
 *
 * @param [out] packet - empty packet to fill.
 * @param [in] buffer - buffer to read from.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtp_packet_parse(rtp_packet *packet, const uint8_t *buffer, size_t size);

/**
 * @brief Set the RTP packet payload.
 *
 * Allocates a new buffer and initializes it with the passed in data. If a
 * payload buffer already exists then this method will return -1.
 *
 * @param [out] packet - packet to set on.
 * @param [in] data - payload data.
 * @param [in] size - payload data size.
 * @return 0 on success.
 */
int rtp_packet_set_payload(rtp_packet *packet, const void *data, size_t size);

/**
 * @brief Clear the RTP packet payload.
 *
 * @param [out] packet - packet to clear on.
 */
void rtp_packet_clear_payload(rtp_packet *packet);

#endif // RTP_PACKET_H_
