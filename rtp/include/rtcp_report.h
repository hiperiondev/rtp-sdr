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

#ifndef RTCP_REPORT_H_
#define RTCP_REPORT_H_

#include <stdint.h>
#include <stddef.h>

#include "rtp_ntp.h"
#include "rtp_source.h"

/**
 * @brief RTCP SR/RR report.
 */
typedef struct rtcp_report {
    uint32_t ssrc; /**< Source identifier. */
    unsigned int fraction :8; /**< Fraction lost since last SR/RR. */
    int lost :24; /**< Cumulative number of packets lost. */
    uint32_t last_seq; /**< Highest sequence number received. */
    uint32_t jitter; /**< Interarrival jitter. */
    uint32_t lsr; /**< Last SR. */
    uint32_t dlsr; /**< Delay since last SR. */
} rtcp_report;

/**
 * @brief Initialize a report.
 *
 * @param [out] report - report to initialize.
 * @param [in] s - source state information.
 * @param [in] tc - the current time.
 */
void rtcp_report_init(rtcp_report *report, rtp_source *s, ntp_tv tc);

/**
 * @brief Write a report to a buffer.
 *
 * @param [in] report - report to serialize.
 * @param [out] buffer - buffer to write to.
 * @param [in] size - buffer size.
 * @return number of bytes written or -1 on failure.
 */
int rtcp_report_serialize(const rtcp_report *report, uint8_t *buffer, size_t size);

/**
 * @brief Parse a report from a buffer.
 *
 * @param [out] report - empty report to fill.
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return 0 on success.
 */
int rtcp_report_parse(rtcp_report *report, const uint8_t *buffer, size_t size);

#endif // RTCP_REPORT_H_
