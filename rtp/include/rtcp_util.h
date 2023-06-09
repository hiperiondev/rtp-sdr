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

#ifndef RTCP_UTIL_H_
#define RTCP_UTIL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @enum RTCP_ERROR
 * @brief rtcp error
 *
 */
enum RTCP_ERROR {
    RTCP_OK    = 0, /**< OK */   /**< RTCP_OK */
    RTCP_ERROR = -1 /**< ERROR *//**< RTCP_ERROR */
};

/**
 * @brief The minimum average time between RTCP packets in seconds.
 *
 * @see IETF RFC3550 "Computing the RTCP Transmission Interval" (§A.7)
 *
 * This time prevents the reports from `clumping' when sessions are small and
 * the law of large numbers isn't helping to smooth out the traffic.  It also
 * keeps the report interval from becoming ridiculously small during transient
 * outages like a network partition.
 */
#ifndef LIBRTP_RTCP_MIN_TIME
#define LIBRTP_RTCP_MIN_TIME (5.0)
#endif

/**
 * @brief The fraction of the RTCP brandwidth reserved for senders.
 *
 * @see IETF RFC3550 "Computing the RTCP Transmission Interval" (§A.7)
 *
 * This fraction was chosen so that in a typical session with one or two active
 * senders, the computed report time would be roughly equal to the minimum
 * report time so that we don't unnecessarily slow down receiver reports.
 */
#ifndef LIBRTP_RTCP_SENDER_BW_FRACTION
#define LIBRTP_RTCP_SENDER_BW_FRACTION (0.25)
#endif

/**
 * @brief Parses the RTCP packet type from a raw buffer.
 *
 * Use this to determine what type of RTCP packet has been received before
 * continuing on to more specific parsing.
 *
 * @param [in] buffer - buffer to parse.
 * @param [in] size - buffer size.
 * @return packet type or -1 on failure.
 */
int rtcp_type(const uint8_t *buffer, size_t size);

/**
 * @brief Calculates the RTCP transmission interval in seconds.
 *
 * @see IETF RFC3550 "Computing the RTCP Transmission Interval" (§A.7)
 *
 * @param [in] members - the current estimate for the number of session members.
 * @param [in] senders - the current estimate for the number of session senders.
 * @param [in] rtcp_bw - the target RTCP bandwidth, i.e. the total bandwidth
 *  that will be used by all members of this session (in bits/s). Note that this
 *  should be some fraction of the total bandwidth, typically 5%.
 * @param [in] we_sent - true if the application has sent data since the 2nd
 *  previous RTCP report was transmitted.
 * @param [in] avg_rtcp_size - the average compound RTCP packet size, in octets,
 *  over all RTCP packets sent and received by this participant. The size
 *  should include lower-layer transport and network protocol headers.
 * @param [in] initial - true if the application has not yet sent an RTCP packet.
 * @return Deterministic calculated interval in seconds.
 */
double rtcp_interval(int members, int senders, double rtcp_bw, bool we_sent, double avg_rtcp_size, bool initial);

/**
 * @brief Recompute the next RTCP packet transmission time.
 *
 * This function provides an implementation of the "reverse reconsideration"
 * algorithm for recomputing RTCP transmission time due to a change in the
 * number of session members (e.g., from receiving BYE packet).
 *
 * @see IETF RFC3550 "Receiving an RTCP BYE Packet" (§6.3.4)
 *
 * @param [in,out] tp - the last time an RTCP packet was transmitted.
 * @param [in,out] tn - the next scheduled transmission time of an RTCP packet.
 * @param [in] tc - the current time.
 * @param [in] pmembers - the estimated number of session members at the time
 *  tn was last computed.
 * @param [in] members - the most current estimate for the number of session
 *  members.
 */
void rtcp_reverse_reconsider(double *tp, double *tn, double tc, int pmembers, int members);

#endif // RTCP_UTIL_H_
