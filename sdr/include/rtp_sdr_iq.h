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

/**
 * @enum RTP_SDR_ERROR
 * @brief rtp_sdr error types
 *
 */
enum RTP_SDR_ERROR {
    RTP_SDR_OK = 0,    /**< OK */
    RTP_SDR_ERROR = -1 /**< ERROR */
};

/**
 * @enum IQ_TYPE
 * @brief i/q type data type
 *
 */
typedef enum IQ_TYPE {
    IQ_PT8  = 97, /**< NON-standard payload type for raw I/Q stream - signed 8 bit version */
    IQ_PT16 = 98, /**< NON-standard payload type for raw I/Q stream - signed 16 bit version */
    IQ_PT24 = 99, /**< NON-standard payload type for raw I/Q stream - signed 24 bit version */
    IQ_PT32 = 100 /**< NON-standard payload type for raw I/Q stream - signed 32 bit version */
} iq_type_t;      /**< i/q type data type */

/**
 * @enum SAMPLE_RATE
 * @brief sample rate
 *
 */
typedef enum SAMPLE_RATE {
    SR_48K   = 48000,  /**< sample rate 48000 bps */
    SR_96K   = 96000,  /**< sample rate 96000 bps */
    SR_192K  = 192000, /**< sample rate 192000 bps */
    SR_384K  = 384000, /**< sample rate 384000 bps */
    SR_768K  = 768000, /**< sample rate 768000 bps */
    SR_1536K = 1536000 /**< sample rate 1536000 bps */
} sample_rate_t;       /**< sample rate data type */

/**
 * @struct session_iq_s
 * @brief i/q session
 *
 */
typedef struct session_iq_s {
             bool tx_enabled;       /**< enable tx */
             bool use_fec;          /**< use fec correction frame */
       rtp_header *tx_header;       /**< tx rtp header */
       rtp_header *rx_header;       /**< rx rtp header */
          int32_t tx_frame_samples; /**< tx samples per frame */
          int32_t rx_frame_samples; /**< rx samples per frame */
         uint32_t frame_size;       /**< frame size */
    rbuf_handle_t tx_iq_buffer;     /**< tx i/q circular buffer */
    rbuf_handle_t rx_iq_buffer;     /**< rx i/q circular buffer */
        iq_type_t type;             /**< rtp payload type (with marker stripped) */
          uint8_t tx_qty;           /**< quantity of transmitters */
          uint8_t rx_qty;           /**< quantity of receivers */
           double *tx_frequency;    /**< tx lo frequency */
           double *rx_frequency;    /**< rx lo frequency */
    sample_rate_t tx_sample_rate;   /**< tx nominal sampling rate */
    sample_rate_t rx_sample_rate;   /**< rx nominal sampling rate */
         uint16_t port;             /**< port */
         uint32_t host;             /**< ip */
     rtp_socket_t socket;           /**< socket */
} *session_iq_t;                    /**< i/q session data type */

/**
 * @fn uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t tx_sample_rate, sample_rate_t rx_sample_rate, uint32_t duration, uint32_t host,
        uint16_t port, bool use_fec, iq_t *tx_buffer, iq_t *rx_buffer, size_t buffer_size, uint8_t tx_qty, uint8_t rx_qty)
 * @brief
 *
 * @param session
 * @param type
 * @param frequency
 * @param tx_sample_rate
 * @param rx_sample_rate
 * @param duration
 * @param host
 * @param port
 * @param use_fec
 * @param tx_buffer
 * @param rx_buffer
 * @param buffer_size
 * @param tx_qty
 * @param rx_qty
 * @return
 */
uint8_t rcp_iq_init(session_iq_t *session, iq_type_t type, double frequency, sample_rate_t tx_sample_rate, sample_rate_t rx_sample_rate, uint32_t duration, uint32_t host,
        uint16_t port, bool use_fec, iq_t *tx_buffer, iq_t *rx_buffer, size_t buffer_size, uint8_t tx_qty, uint8_t rx_qty);

/**
 * @fn void rcp_iq_deinit(session_iq_t *session)
 * @brief
 *
 * @param session
 */
void rcp_iq_deinit(session_iq_t *session);

/**
 * @fn uint8_t rcp_iq_transmit(session_iq_t *session)
 * @brief
 *
 * @param session
 * @param data
 * @return
 */
uint8_t rcp_iq_transmit(session_iq_t *session);

/**
 * @fn uint8_t rcp_iq_receive(session_iq_t *session)
 * @brief
 *
 * @param session
 * @param data
 * @return
 */
uint8_t rcp_iq_receive(session_iq_t *session);

/**
 * @fn void* rcp_iq_transmit_handler(void *arg)
 * @brief
 *
 * @param arg
 */
void* rcp_iq_transmit_handler(void *arg);

/**
 * @fn void* rcp_iq_receive_handler(void *arg)
 * @brief
 *
 * @param arg
 */
void* rcp_iq_receive_handler(void *arg);


#endif /* RTP_IQ_H_ */
