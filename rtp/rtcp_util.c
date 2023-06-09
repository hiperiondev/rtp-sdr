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

#include <stdlib.h>
#include <assert.h>

#include "rtcp_header.h"
#include "rtcp_util.h"

int rtcp_type(const uint8_t *buffer, size_t size) {
    assert(buffer != NULL);

    if (size < 2)
        return RTCP_ERROR;

    int pt = buffer[1];
    if (pt >= RTCP_SR && pt <= RTCP_APP)
        return pt;

    return RTCP_ERROR;
}

double rtcp_interval(int members, int senders, double rtcp_bw, bool we_sent, double avg_rtcp_size, bool initial) {
    const double MIN_TIME = LIBRTP_RTCP_MIN_TIME;
    const double SENDER_BW_FRACTION = LIBRTP_RTCP_SENDER_BW_FRACTION;
    const double RCVR_BW_FRACTION = (1.0 - SENDER_BW_FRACTION);

    /*
     * To compensate for "timer reconsideration" converging to a
     * value below the intended average.
     */
    static const double COMPENSATION = 2.71828 - 1.5;

    /*
     * Very first call at application start-up uses half the min
     * delay for quicker notification while still allowing some time
     * before reporting for randomization and to learn about other
     * sources so the report interval will converge to the correct
     * interval more quickly.
     */
    double min_time = MIN_TIME;
    if (initial)
        min_time /= 2;

    /*
     * Dedicate a fraction of the RTCP bandwidth to senders unless
     * the number of senders is large enough that their share is
     * more than that fraction.
     */
    int n = members;
    if (senders <= members * SENDER_BW_FRACTION) {
        if (we_sent) {
            rtcp_bw *= SENDER_BW_FRACTION;
            n = senders;
        }
        else {
            rtcp_bw *= RCVR_BW_FRACTION;
            n -= senders;
        }
    }

    /*
     * The effective number of sites times the average packet size is
     * the total number of octets sent when each site sends a report.
     * Dividing this by the effective bandwidth gives the time
     * interval over which those packets must be sent in order to
     * meet the bandwidth target, with a minimum enforced.  In that
     * time interval we send one report so this time is also our
     * average time between reports.
     */
    double t = avg_rtcp_size * n / rtcp_bw;
    if (t < min_time)
        t = min_time;

    /*
     * To avoid traffic bursts from unintended synchronization with
     * other sites, we then pick our actual next report interval as a
     * random number uniformly distributed between 0.5*t and 1.5*t.
     */
    t *= ((double) (rand()) / RAND_MAX) + 0.5;
    t /= COMPENSATION;

    return t;
}

void rtcp_reverse_reconsider(double *tp, double *tn, double tc, int pmembers, int members) {
    *tn = tc + ((double) members / pmembers) * ((*tn) - tc);
    *tp = tc - ((double) members / pmembers) * (tc - (*tp));
}
