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
// from: https://github.com/embeddedartistry/embedded-resources

#ifndef RTP_SDR_RBUF_H_
#define RTP_SDR_RBUF_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @enum RTP_SDR_RBUF_ERROR
 * @brief buffer errors
 *
 */
enum RTP_SDR_RBUF_ERROR {
    RTP_SDR_RBUF_OK    = 0,/**< RTP_SDR_RBUF_OK */
    RTP_SDR_RBUF_ERROR = -1/**< RTP_SDR_RBUF_ERROR */
};

/**
 * @enum RTP_SDR_RBUF_TYPE
 * @brief buffer type
 *
 */
typedef enum RTP_SDR_RBUF_TYPE {
    RTP_SDR_RBUF_S8,   /**< signed 8 bits buffer */
    RTP_SDR_RBUF_S16,  /**< signed 16 bits buffer */
    RTP_SDR_RBUF_S24,  /**< signed 24 bits buffer */
    RTP_SDR_RBUF_S32   /**< signed 32 bits buffer */
} rtp_sdr_sbuf_type_t; /**< buffer type data type */

/**
 * @struct iq_s
 * @brief i/q data
 *
 */
typedef struct iq_s {
    union {
         int8_t s8;      /**< signed 8 bits data */
        int16_t s16;     /**< signed 16 bits data */
        int32_t s24_s32; /**< signed 24/32 bits data */
    } i;                 /**< i component */
    union {
         int8_t s8;      /**< signed 8 bits data */
        int16_t s16;     /**< signed 16 bits data */
        int32_t s24_s32; /**< signed 24/32 bits data */
    } q;                 /**< q component */
} iq_t;                  /**< i/q data type */

typedef struct rbuf_s rbuf_t;  /**< opaque circular buffer structure */
typedef rbuf_t *rbuf_handle_t; /**< handle type, the way users interact with the API */

/**
 * @fn rbuf_handle_t rtp_sdr_rbuf_init(iq_t *buffer, size_t size, rtp_sdr_sbuf_type_t type)
 * @brief Pass in a storage buffer and size, returns a circular buffer handle
 *        Requires: buffer is not NULL, size > 0 (size > 1 for the thread safe version, because it holds size - 1 elements)
 *        Ensures: me has been created and is returned in an empty state
 *
 * @param buffer
 * @param size
 * @param type
 * @return
 */
rbuf_handle_t rtp_sdr_rbuf_init(iq_t *buffer, size_t size, rtp_sdr_sbuf_type_t type);

/**
 * @fn void rtp_sdr_rbuf_free(rbuf_handle_t me)
 * @brief Free a circular buffer structure
 *        Requires: me is valid and created by circular_buf_init
 *        Does not free data buffer; owner is responsible for that
 *
 * @param me
 */
void rtp_sdr_rbuf_free(rbuf_handle_t me);

/**
 * @fn void rtp_sdr_rbuf_reset(rbuf_handle_t me)
 * @brief Reset the circular buffer to empty, head == tail. Data not cleared
 *        Requires: me is valid and created by circular_buf_init
 *
 * @param me
 */
void rtp_sdr_rbuf_reset(rbuf_handle_t me);

/**
 * @fn void rtp_sdr_rbuf_put(rbuf_handle_t me, iq_t data)
 * @brief Put that continues to add data if the buffer is full. Old data is overwritten
 *        Note: if you are using the thread safe version, this API cannot be used, because it modifies the tail pointer in some cases.
 *        Use circular_buf_try_put instead.
 *        Requires: me is valid and created by circular_buf_init
 *
 * @param me
 * @param data
 */
void rtp_sdr_rbuf_put(rbuf_handle_t me, iq_t data);

/**
 * @fn int rtp_sdr_rbuf_try_put(rbuf_handle_t me, iq_t data)
 * @brief Put that rejects new data if the buffer is full
 *        Note: if you are using the thread safe version, *this* is the put you should use
 *        Requires: me is valid and created by circular_buf_init
 *        Returns 0 on success, -1 if buffer is full
 *
 * @param me
 * @param data
 * @return
 */
int rtp_sdr_rbuf_try_put(rbuf_handle_t me, iq_t data);

///
/**
 * @fn int rtp_sdr_rbuf_get(rbuf_handle_t me, iq_t *data)
 * @brief Retrieve a value from the buffer
 *        Requires: me is valid and created by circular_buf_init
 *        Returns 0 on success, -1 if the buffer is empty
 *
 * @param me
 * @param data
 * @return
 */
int rtp_sdr_rbuf_get(rbuf_handle_t me, iq_t *data);

/**
 * @fn bool rtp_sdr_rbuf_empty(rbuf_handle_t me)
 * @brief Checks if the buffer is empty
 * Requires: me is valid and created by circular_buf_init
 * Returns true if the buffer is empty
 *
 * @param me
 * @return
 */
bool rtp_sdr_rbuf_empty(rbuf_handle_t me);

/**
 * @fn bool rtp_sdr_rbuf_full(rbuf_handle_t me)
 * @brief Checks if the buffer is full
 *        Requires: me is valid and created by circular_buf_init
 *        Returns true if the buffer is full
 *
 * @param me
 * @return
 */
bool rtp_sdr_rbuf_full(rbuf_handle_t me);

/**
 * @fn size_t rtp_sdr_rbuf_capacity(rbuf_handle_t me)
 * @brief Check the capacity of the buffer
 *        Requires: me is valid and created by circular_buf_init
 *        Returns the maximum capacity of the buffer
 *
 * @param me
 * @return
 */
size_t rtp_sdr_rbuf_capacity(rbuf_handle_t me);

/**
 * @fn size_t rtp_sdr_rbuf_size(rbuf_handle_t me)
 * @brief Check the number of elements stored in the buffer
 *        Requires: me is valid and created by circular_buf_init
 *        Returns the current number of elements in the buffer
 *
 * @param me
 * @return
 */
size_t rtp_sdr_rbuf_size(rbuf_handle_t me);

/**
 * @fn int rtp_sdr_rbuf_peek(rbuf_handle_t me, iq_t *data, unsigned int look_ahead_counter)
 * @brief Look ahead at values stored in the circular buffer without removing the data
 *        Requires:
 *        - me is valid and created by circular_buf_init
 *        - look_ahead_counter is less than or equal to the value returned by circular_buf_size()
 *        Returns 0 if successful, -1 if data is not available
 *
 * @param me
 * @param data
 * @param look_ahead_counter
 * @return
 */
int rtp_sdr_rbuf_peek(rbuf_handle_t me, iq_t *data, unsigned int look_ahead_counter);

#endif // RTP_SDR_RBUF_H_
