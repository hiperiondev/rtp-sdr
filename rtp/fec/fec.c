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

#ifdef DEBUG
#include <stdio.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "fec.h"
#include "fec_matrix.h"

// Free a FEC parameter structure.
void fec_free(fec_t *fec) {
    assert(fec != NULL);
    assert(fec->gen_matrix != NULL);
    free(fec->gen_matrix);
    free(fec);
}

// Initialize a FEC parameter structure. Create a generator matrix.
fec_t* fec_new(unsigned int k, unsigned int n) {
    assert((k <= n) || "k is too big");
    assert((k <= 256) || "k is too big");
    assert((n <= 256) || "n is too big");

    // Init Galois arithmetic if not already initialized.
    static int gf_initialized = 0;
    if (!gf_initialized) {
        gf_init();
        gf_initialized = 1;
    }

    fec_t *res;
    res = malloc(sizeof(fec_t));
    assert(res != NULL);
    res->gen_matrix = malloc(sizeof(gf) * k * n);
    assert(res->gen_matrix != NULL);

    res->k = k;
    res->n = n;

    // Fill the matrix with powers of field elements.
    gf tmp[k * n];
    //gf *tmp = res->gen_matrix;

    // First row is special (powers of 0).
    tmp[0] = 1;
    unsigned int col;
    for (col = 1; col < k; col++)
        tmp[col] = 0;

    gf *p;
    unsigned int row;
    for (p = tmp + k, row = 0; row < n - 1; row++, p += k) {
        for (col = 0; col < k; col++)
            p[col] = gf_polys[(row * col) % 255];
    }

#ifdef DEBUG
    fprintf(stderr, "first vandermonde matrix\n");
    matrix_print(tmp, res->n, res->k);
#endif

    // Invert the upper k times k vandermonde matrix.
    matrix_inv_vandermonde(tmp, k);

#ifdef DEBUG
    fprintf(stderr, "\ninverted vandermonde matrix\n");
    matrix_print(tmp, res->n, res->k);
#endif

    // Multiply the inverted upper k times k vandermonde matrix with the lower band of the matrix.
    matrix_mul(tmp + k * k, tmp, res->gen_matrix + k * k, n - k, k, k);

    // Fill the upper k times k matrix with the identity matrix to generate a systematic matrix.
    for (row = 0; row < k; row++)
        for (col = 0; col < k; col++)
            if (col == row)
                res->gen_matrix[row * k + col] = 1;
            else
                res->gen_matrix[row * k + col] = 0;

#ifdef DEBUG
    fprintf(stderr, "\ngenerated matrix\n");
    matrix_print(res->gen_matrix, res->n, res->k);
#endif

    return res;
}

// Produce encoded output packet.

// Encodes the idx'th output data packet from the k data packets in src and the generator matrix in fec.
// For idx < k, we just copy the data (systematic matrix).
void fec_encode(fec_t *fec, gf *src[], gf *dst, unsigned int idx, unsigned int len) {
    assert((idx < fec->n) || "Index of output packet to high");

    if (idx < fec->k) {
        memcpy(dst, src[idx], len * sizeof(gf));
    }
    else {
        gf *p = fec->gen_matrix + idx * fec->k;

        bzero(dst, len * sizeof(gf));
        unsigned int i;
        for (i = 0; i < fec->k; i++)
            gf_add_mul(dst, src[i], p[i], len);
    }
}

// Builds the decoding matrix.
// Builds the decoding matrix into matrix out of the indexes stored in idxs.
// Returns 0 on error, 1 on success.
int fec_decode_matrix(fec_t *fec, gf *matrix, unsigned int idxs[]) {
    gf *p;

    unsigned int i;
    for (p = matrix, i = 0; i < fec->k; i++, p += fec->k) {
        assert((idxs[i] < fec->n) || "index of packet to high for FEC");

        memcpy(p, fec->gen_matrix + idxs[i] * fec->k, fec->k * sizeof(gf));
    }

#ifdef DEBUG
    matrix_print(matrix, fec->k, fec->k);
#endif

    if (!matrix_inv(matrix, fec->k))
        return 0;

    return 1;
}

// Put straight packets at the right place.
// Packets with index < k are put at the right place.
static int fec_shuffle(fec_t *fec, unsigned int idxs[]) {
    unsigned int i;
    for (i = 0; i < fec->k;) {
        if ((idxs[i] >= fec->k) || (idxs[i] == i)) {
            i++;
        }
        else {
            unsigned int c = idxs[i];

            // check for conflicts
            if (idxs[c] == c)
                return 0;

            idxs[i] = idxs[c];
            idxs[c] = c;
        }
    }

    return 1;
}

// Decode the received packets.
int fec_decode(fec_t *fec, gf *pkts, unsigned int idxs[], unsigned len) {
    assert(fec != NULL);

    if (!fec_shuffle(fec, idxs))
        return 0;

    // Build decoding matrix.
    gf dec_matrix[fec->k * fec->k];
    if (!fec_decode_matrix(fec, dec_matrix, idxs))
        return 0;

    unsigned int row;
    for (row = 0; row < fec->k; row++) {
        if (idxs[row] >= fec->k) {
            gf *pkt = pkts + row * len;

            bzero(pkt, len * sizeof(gf));
            unsigned int col;
            for (col = 0; col < fec->k; col++) {
                gf_add_mul(pkt, pkts + idxs[col] * len, dec_matrix[row * fec->k + col], len);
            }
        }
    }

    return 1;
}

#ifdef FEC_TEST
#include <stdio.h>

void testit(char *name, unsigned int result, unsigned int should) {
    if (result == should) {
        printf("Test %s was successful\n", name);
    }
    else {
        printf("Test %s was not successful, %u should have been %u\n", name, result, should);
    }
}

int main(void) {
    fec_t *fec;

    gf_init();
    fec = fec_new(4, 8);
    printf("\n");

    gf src_pkts[4][4] = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 }, { 13, 14, 15, 16 } };
    gf dst_pkts[8 * 4];
    gf *src_ptrs[4] = { src_pkts[0], src_pkts[1], src_pkts[2], src_pkts[3] };
    unsigned int idxs[4] = { 3, 5, 1, 0 }; /* from 0 ?? */

    int i;
    for (i = 0; i < 8; i++) {
        fec_encode(fec, src_ptrs, dst_pkts + i * 4, i, 4);

        int j;
        for (j = 0; j < 4; j++)
            printf("%u ", dst_pkts[i * 4 + j]);
        printf("\n");
    }

    memset(dst_pkts + 2 * 4, 0, 4);
    memset(dst_pkts + 4 * 4, 0, 4);
    memset(dst_pkts + 6 * 4, 0, 4);
    memset(dst_pkts + 7 * 4, 0, 4);

    testit("fec decode", fec_decode(fec, dst_pkts, idxs, 4), 1);

    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 4; j++)
            testit("fec decode", dst_pkts[i * 4 + j], src_pkts[i][j]);
    }

    fec_free(fec);

    return 0;
}
#endif /* FEC_TEST */

