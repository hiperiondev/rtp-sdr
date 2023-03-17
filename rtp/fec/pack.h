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

#ifndef PACK_H__
#define PACK_H__

/*M
 \emph{8 bits value packing macro.}

 This macro advances the buffer pointer it is given as first
 argument, and fills the buffer with the 8 bits value
 given as second argument.
 **/
#define UINT8_PACK(ptr, i) { *(ptr++) = i; }

/*M
 \emph{8 bits value unpacking macro.}

 This macro advances the buffer pointer it is given as first
 argument, and returns the unpacked 8 bits value in the buffer.
 **/
#define UINT8_UNPACK(ptr) (*(ptr++))

/*M
 \emph{16 bits value packing macro.}

 This macro advances the buffer pointer it is given as first
 argument, and fills the buffer with the big endian packed value
 given as second argument.
 **/
#define UINT16_PACK(ptr, i)                         \
  { *(ptr++) = (unsigned char)(((i) >> 8) & 0xFF);  \
    *(ptr++) = (unsigned char)((i)        & 0xFF); }

/*M
 \emph{32 bits value packing macro.}

 This macro advances the buffer pointer it is given as first
 argument, and fills the buffer with the big endian packed value
 given as second argument.
 **/
#define UINT32_PACK(ptr, i)                         \
  { *(ptr++) = (unsigned char)(((i) >> 24) & 0xFF); \
    *(ptr++) = (unsigned char)(((i) >> 16) & 0xFF); \
    *(ptr++) = (unsigned char)(((i) >> 8)  & 0xFF); \
    *(ptr++) = (unsigned char)((i)         & 0xFF); }

/*M
 \emph{24 bits value packing macro.}
 **/
#define UINT24_PACK(ptr, i)                         \
  { *(ptr++) = (unsigned char)(((i) >> 16) & 0xFF); \
    *(ptr++) = (unsigned char)(((i) >> 8)  & 0xFF); \
    *(ptr++) = (unsigned char)((i)         & 0xFF); }

/*M
 \emph{16 bits value unpacking macro.}

 This macro advances the buffer pointer it is given as first
 argument, and returns the unpacked big endian value in the buffer.
 **/
#define UINT16_UNPACK(ptr) uint16_unpack__(&ptr)
unsigned int uint16_unpack__(/*@out@*/unsigned char **ptr);

/*M
 \emph{32 bits value unpacking macro.}

 This macro advances the buffer pointer it is given as first
 argument, and returns the unpacked big endian value in the
 buffer.
 **/
#define UINT32_UNPACK(ptr) uint32_unpack__(&ptr)
unsigned int uint32_unpack__(/*@out@*/unsigned char **ptr);

/*M
 \emph{16 bits value packing macro.}

 This macro advances the buffer pointer it is given as first
 argument, and fills the buffer with the little endian packed value
 given as second argument.
 **/
#define LE_UINT16_PACK(ptr, i)                      \
  { *(ptr++) = (unsigned char)((i) & 0xFF);         \
    *(ptr++) = (unsigned char)(((i) >> 8) & 0xFF); }

/*M
 \emph{32 bits value packing macro.}

 This macro advances the buffer pointer it is given as first
 argument, and fills the buffer with the little endian packed value
 given as second argument.
 **/
#define LE_UINT32_PACK(ptr, i)                      \
  { *(ptr++) = (unsigned char)((i) & 0xFF);         \
    *(ptr++) = (unsigned char)(((i) >> 8)  & 0xFF); \
    *(ptr++) = (unsigned char)(((i) >> 16) & 0xFF); \
    *(ptr++) = (unsigned char)(((i) >> 24) & 0xFF); }

/*M
 \emph{16 bits value unpacking macro.}

 This macro advances the buffer pointer it is given as first
 argument, and returns the unpacked little endian value in the buffer.
 **/
#define LE_UINT16_UNPACK(ptr) le_uint16_unpack__(&ptr)
unsigned int le_uint16_unpack__(/*@out@*/unsigned char **ptr);

/*M
 \emph{32 bits value unpacking macro.}

 This macro advances the buffer pointer it is given as first
 argument, and returns the unpacked little endian value in the
 buffer.
 **/
#define LE_UINT32_UNPACK(ptr) le_uint32_unpack__(&ptr)
unsigned int le_uint32_unpack__(/*@out@*/unsigned char **ptr);

#endif /* PACK_H__ */

/*C
 **/
