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

#include "fec_galois.h"

// Polynomial representation of field elements.
gf gf_polys[256] = { 0 };

// Logarithmic representation of field elements.
gf gf_logs[256] = { 0 };

// Precomputed multiplication table.
gf gf_mul[256][256] = { { 0 } };

// Precomputed inverse table.
gf gf_inv[256] = { 0 };

// A primitive polynomial.

// A primitive polynomial for \gf{2^8}, namely 1 + x^2 + x^3 + x^4 + x^8
static char gf_prim_poly[] = "101110001";

// Initialize data structures.}
void gf_init(void) {
    // Notice that $x^8 = x^4 + x^3 + x^2 + 1$.
    // We will fill up the "eight" power of alpha from the prime polynomial.
    gf_polys[8] = 0;

    // The first 8 elements are just alpha shifted to the left.
    int i;
    gf g = 1;
    for (i = 0; i < 8; i++, g <<= 1) {
        gf_polys[i] = g;

        // Remember logarithm by storing it into the logarithm lookup table.
        gf_logs[gf_polys[i]] = i;

        // Fill up the eighth element.
        if (gf_prim_poly[i] == '1')
            gf_polys[8] |= g;
    }
    // Remember logarithm of eight element.
    gf_logs[gf_polys[8]] = 8;

    // For each further element, $a^n = a^(n-1) * a$.
    // We just need to calculate the modulo \verb|gf_prim_poly|, which is of degree $8$.
    g = 1 << 7;
    for (i = 9; i < 255; i++) {
        if (gf_polys[i - 1] >= g)
            /*M
             // $a^{n-1} * a > $ \verb|gf_prim_poly|, then
             // $a^n = a^{n-1} * a = a^8 + ... = a^4 + a^3 + a^2 + 1$.
             **/
            gf_polys[i] = gf_polys[8] ^ ((gf_polys[i - 1]) << 1);
        else
            gf_polys[i] = gf_polys[i - 1] << 1;

        // Remember logarithm.
        gf_logs[gf_polys[i]] = i;
    }

    // The 0th element is undefined.
    gf_logs[0] = 0xFF;

    // Compute multiplication table.
    for (i = 0; i < 256; i++) {
        int j;
        for (j = 0; j < 256; j++) {
            if ((i == 0) || (j == 0))
                gf_mul[i][j] = 0;
            else
                gf_mul[i][j] = gf_polys[(gf_logs[i] + gf_logs[j]) % 255];
        }

        for (j = 0; j < 256; j++)
            gf_mul[0][j] = gf_mul[j][0] = 0;
    }

    // Compute inverses.
    gf_inv[0] = 0;
    gf_inv[1] = 1;
    for (i = 2; i < 256; i++)
        gf_inv[i] = gf_polys[255 - gf_logs[i]];
}

// Computes addition of a row multiplied by a constant.
// Computes $a = a + c * b$, $a, b \in \gf{2^8}^k, c \in \gf{2^8}$.
void gf_add_mul(gf *a, gf *b, gf c, int k) {
    int i;
    for (i = 0; i < k; i++)
        a[i] = GF_ADD(a[i], GF_MUL(c, b[i]));
}

#ifdef GALOIS_TEST
#include <stdio.h>

void testit(char *name, int result, int should) {
    if (result == should) {
        printf("Test %s was successful\n", name);
    }
    else {
        printf("Test %s was not successful, %x should have been %x\n", name, result, should);
    }
}

int main(void) {
    gf a, b, c;

    gf_init();
    a = 1;
    b = 37;
    c = 78;
    testit("1 * ( 37 + 78 ) = 1 * 37 + 1 * 78", GF_MUL(a, GF_ADD(b, c)), GF_ADD(GF_MUL(a, b), GF_MUL(a, c)));
    testit("(1 * 37) * 78 = 1 * (37 * 78)", GF_MUL(GF_MUL(a, b), c), GF_MUL(a, GF_MUL(b, c)));
    testit("(37 * 78) * 37 = (37 * 37) * 78", GF_MUL(GF_MUL(b, c), b), GF_MUL(GF_MUL(b, b), c));
    testit("b * b^-1 = 1", GF_MUL(b, GF_INV(b)), 1);

    return 0;
}

#endif /* GALOIS_TEST */
