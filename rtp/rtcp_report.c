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
#include <string.h>
#include <assert.h>

#include "rtp_ntp.h"
#include "rtp_util.h"
#include "rtcp_util.h"
#include "rtcp_report.h"

void rtcp_report_init(rtcp_report *report, rtp_source *s, ntp_tv tc) {
    assert(report != NULL);
    assert(s != NULL);

    report->ssrc = s->id;
    report->fraction = s->fraction;
    report->lost = s->lost;
    report->last_seq = s->max_seq;
    report->jitter = (uint32_t) s->jitter;
    report->lsr = ntp_short(s->lsr);
    if (report->lsr && (tc.sec || tc.frac))
        report->dlsr = ntp_short(ntp_diff(tc, s->lsr));
}

int rtcp_report_serialize(const rtcp_report *report, uint8_t *buffer, size_t size) {
    assert(report != NULL);
    assert(buffer != NULL);

    if (size < 24)
        return RTCP_ERROR;

    write_u32(buffer, report->ssrc);

    buffer[4] = report->fraction;
    write_s24_s32(buffer + 5, report->lost);

    write_u32(buffer + 8, report->last_seq);
    write_u32(buffer + 12, report->jitter);
    write_u32(buffer + 16, report->lsr);
    write_u32(buffer + 20, report->dlsr);

    return 24;
}

int rtcp_report_parse(rtcp_report *report, const uint8_t *buffer, size_t size) {
    assert(report != NULL);
    assert(buffer != NULL);

    if (size < 24)
        return RTCP_ERROR;

    report->ssrc = read_u32(buffer);

    report->fraction = buffer[4];
    report->lost = read_s24(buffer + 5);
    report->last_seq = read_u32(buffer + 8);
    report->jitter = read_u32(buffer + 12);
    report->lsr = read_u32(buffer + 16);
    report->dlsr = read_u32(buffer + 20);

    return RTCP_OK;
}
