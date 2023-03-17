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

#ifndef RTP_IQ_H_
#define RTP_IQ_H_

#include <stdint.h>
#include <stdbool.h>

#include "rtp_header.h"
#include "rtp_socket.h"
#include "rtp_sdr_rbuf.h"

typedef enum IQ_TYPE {
    IQ_PT8  = 97, // NON-standard payload type for raw I/Q stream - signed 8 bit version
    IQ_PT16 = 98, // NON-standard payload type for raw I/Q stream - signed 16 bit version
    IQ_PT24 = 99  // NON-standard payload type for raw I/Q stream - signed 24 bit version
} iq_type_t;

typedef enum SAMPLE_RATE {
    SR_48K   = 48000,
    SR_96K   = 96000,
    SR_192K  = 192000,
    SR_384K  = 384000,
    SR_768K  = 768000,
    SR_1536K = 1536000
} sample_rate_t;

// One for each session being recorded
typedef struct session_iq_s {
            bool use_fec;
      rtp_header *header;     // RTP header
      union {
           int8_t *_8;
          int16_t *_16;
          int32_t *_24;
      } frame;                 // RTP frame buffer
      rbuf_handle_t iq_buffer; // iq circular buffer;
       iq_type_t type;         // RTP payload type (with marker stripped)
          double frequency;    // rx/tx LO frequency
   sample_rate_t sample_rate;  // nominal sampling rate

        uint16_t port;         // port
        uint32_t host;         // IP

        rtp_socket_t socket;   // socket
} *session_iq_t;

uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t sample_rate, uint32_t duration, uint32_t host, uint16_t port,
        bool use_fec, iq_t *buffer, size_t buffer_size);
   void rcp_iq_deinit(session_iq_t *session);

#endif /* RTP_IQ_H_ */
