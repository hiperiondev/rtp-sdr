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

typedef struct {
    union {
         int8_t s8;
        int16_t s16;
        int32_t s24;
    } i;
    union {
         int8_t s8;
        int16_t s16;
        int32_t s24;
    } q;
} iq_t;

/// Opaque circular buffer structure
typedef struct rbuf_s rbuf_t;

/// Handle type, the way users interact with the API
typedef rbuf_t *rbuf_handle_t;

/// Pass in a storage buffer and size, returns a circular buffer handle
/// Requires: buffer is not NULL, size > 0 (size > 1 for the thread safe
//  version, because it holds size - 1 elements)
/// Ensures: me has been created and is returned in an empty state
rbuf_handle_t rbuf_init(iq_t *buffer, size_t size);

/// Free a circular buffer structure
/// Requires: me is valid and created by circular_buf_init
/// Does not free data buffer; owner is responsible for that
void rbuf_free(rbuf_handle_t me);

/// Reset the circular buffer to empty, head == tail. Data not cleared
/// Requires: me is valid and created by circular_buf_init
void rbuf_reset(rbuf_handle_t me);

/// Put that continues to add data if the buffer is full
/// Old data is overwritten
/// Note: if you are using the thread safe version, this API cannot be used, because
/// it modifies the tail pointer in some cases. Use circular_buf_try_put instead.
/// Requires: me is valid and created by circular_buf_init
void rbuf_put(rbuf_handle_t me, iq_t data);

/// Put that rejects new data if the buffer is full
/// Note: if you are using the thread safe version, *this* is the put you should use
/// Requires: me is valid and created by circular_buf_init
/// Returns 0 on success, -1 if buffer is full
int rbuf_try_put(rbuf_handle_t me, iq_t data);

/// Retrieve a value from the buffer
/// Requires: me is valid and created by circular_buf_init
/// Returns 0 on success, -1 if the buffer is empty
int rbuf_get(rbuf_handle_t me, iq_t *data);

/// CHecks if the buffer is empty
/// Requires: me is valid and created by circular_buf_init
/// Returns true if the buffer is empty
bool rbuf_empty(rbuf_handle_t me);

/// Checks if the buffer is full
/// Requires: me is valid and created by circular_buf_init
/// Returns true if the buffer is full
bool rbuf_full(rbuf_handle_t me);

/// Check the capacity of the buffer
/// Requires: me is valid and created by circular_buf_init
/// Returns the maximum capacity of the buffer
size_t rbuf_capacity(rbuf_handle_t me);

/// Check the number of elements stored in the buffer
/// Requires: me is valid and created by circular_buf_init
/// Returns the current number of elements in the buffer
size_t rbuf_size(rbuf_handle_t me);

/// Look ahead at values stored in the circular buffer without removing the data
/// Requires:
///		- me is valid and created by circular_buf_init
///		- look_ahead_counter is less than or equal to the value returned by circular_buf_size()
/// Returns 0 if successful, -1 if data is not available
int rbuf_peek(rbuf_handle_t me, iq_t *data, unsigned int look_ahead_counter);

#endif // RTP_SDR_RBUF_H_
