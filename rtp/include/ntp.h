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

#ifndef LIBRTP_NTP_H_
#define LIBRTP_NTP_H_

#include <stdint.h>

/**
 * @brief NTP timeval.
 */
typedef struct ntp_tv {
    uint32_t sec;  /**< Seconds since Jan 1, 1900. */
    uint32_t frac; /**< Fractional seconds (2^32). */
} ntp_tv;

/**
 * @brief Converts an NTP timestamp to its double representation.
 *
 * @param ntp - NTP timestamp.
 * @return seconds since Jan 1, 1900.
 */
double ntp_to_double(ntp_tv ntp);

/**
 * @brief Converts an NTP timestamp to a Unix timestamp.
 *
 * @param [in] ntp - NTP timestamp.
 * @return seconds since Jan 1, 1970.
 */
double ntp_to_unix(ntp_tv ntp);

/**
 * @brief Converts a double to an NTP timestamp.
 *
 * @param s - seconds since Jan 1, 1900.
 * @return NTP timestamp.
 */
ntp_tv ntp_from_double(double s);

/**
 * @brief Converts a Unix timestamp to an NTP timestamp.
 *
 * @param [in] s - seconds since Jan 1, 1970.
 * @return NTP timestamp.
 */
ntp_tv ntp_from_unix(double s);

/**
 * @brief Convert an NTP timestamp to its short representation.
 *
 * @param [in] ntp - NTP timestamp.
 * @return NTP short format timestamp.
 */
uint32_t ntp_short(ntp_tv ntp);

/**
 * @brief Returns the difference between two NTP timestamps.
 *
 * @param [in] a - first timestamp.
 * @param [in] b - second timestamp.
 * @return ntp time difference (a - b).
 */
ntp_tv ntp_diff(ntp_tv a, ntp_tv b);

#endif // LIBRTP_NTP_H_
