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

#ifndef RTP_UTIL_H_
#define RTP_UTIL_H_

#include <stdint.h>

/**
 * @enum RTP_ERROR
 * @brief
 *
 */
enum RTP_ERROR {
    RTP_OK    = 0, /**< OK */
    RTP_ERROR = -1 /**< ERROR */
};

/**
 * @brief Write a 24-bit value to a buffer (big-endian).
 *
 * @param [out] buffer - buffer to write to.
 * @param [in] value - value to write.
 * @private
 */
void write_u24(uint8_t *buffer, uint32_t value);

/**
 * @brief Write a signed 24-bit value to a buffer (big endian).
 *
 * @param [out] buffer - buffer to write to.
 * @param [in] value - value to write.
 * @private
 */
void write_s24_s32(uint8_t *buffer, int32_t value);

/**
 * @brief Write a 32-bit value to a buffer (big-endian).
 *
 * @param [out] buffer - buffer to write to.
 * @param [in] value - value to write.
 * @private
 */
void write_u32(uint8_t *buffer, uint32_t value);

/**
 * @brief Read a 24-bit value from a buffer (big-endian).
 *
 * @param [in] buffer - buffer to read from.
 * @return uint32_t - value read.
 * @private
 */
uint32_t read_u24(const uint8_t *buffer);

/**
 * @brief Read a signed 24-bit value from a buffer (big-endian).
 *
 * @param [in] buffer - buffer to read from.
 * @return int32_t - value read.
 * @private
 */
int32_t read_s24(const uint8_t *buffer);

/**
 * @brief Read a 32-bit value from a buffer (big-endian).
 *
 * @param [in] buffer - buffer to read from.
 * @return uint32_t - value read.
 * @private
 */
uint32_t read_u32(const uint8_t *buffer);

/**
 * @brief Write a 16-bit value to a buffer (big-endian).
 *
 * @param [in] buffer - buffer to write to.
 * @param [in] value - value to write.
 * @private
 */
void write_u16(uint8_t *buffer, uint16_t value);

/**
 * @brief Read a 16-bit value from a buffer (big-endian).
 *
 * @param [in] buffer to read from.
 * @return uint16_t - value read.
 * @private
 */
uint16_t read_u16(const uint8_t *buffer);

#endif // RTP_UTIL_H_
