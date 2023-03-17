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

#include "rtp_ntp.h"

/**
 * @brief 1970 - 1900 in seconds.
 * @private
 */
#define LIBRTP_NTP_UNIX_OFFSET (2208988800UL)

/**
 * @brief 2^32 as a double.
 * @private
 */
#define LIBRTP_NTP_FRAC (4294967296.0)

double ntp_to_double(ntp_tv ntp) {
    double s = (double) ntp.sec;
    s += (double) ntp.frac / LIBRTP_NTP_FRAC;

    return s;
}

double ntp_to_unix(ntp_tv ntp) {
    // Shift epoch from 1900 to 1970
    return ntp_to_double(ntp) - LIBRTP_NTP_UNIX_OFFSET;
}

ntp_tv ntp_from_double(double s) {
    ntp_tv ntp;
    ntp.sec = (uint32_t) s;

    double frac = (s - ntp.sec);
    ntp.frac = (uint32_t) (frac * LIBRTP_NTP_FRAC);

    return ntp;
}

ntp_tv ntp_from_unix(double s) {
    // Shift epoch from 1970 to 1900
    return ntp_from_double(s + LIBRTP_NTP_UNIX_OFFSET);
}

uint32_t ntp_short(ntp_tv ntp) {
    // Lower 16 bits of seconds and upper 16 bits of frac.
    return (ntp.sec << 16) | (ntp.frac >> 16);
}

ntp_tv ntp_diff(ntp_tv a, ntp_tv b) {
    double s1 = ntp_to_double(a);
    double s2 = ntp_to_double(b);

    return ntp_from_double(s1 - s2);
}
