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

#include <stdio.h>
#include <string.h>

#include "fec_matrix.h"

// Print a m times n matrix.
void matrix_print(gf *a, int m, int n) {
    int row;
    for (row = 0; row < m; row++) {
        int col;
        for (col = 0; col < n; col++)
            fprintf(stderr, "%-3u ", a[row * n + col]);
        fprintf(stderr, "\n");
    }
}

// Matrix multiplication.
//Computes c = a * b, with a in gf{2^8}^{n times k}, b in gf{2^8}^{k times m}, c in gf{2^8}^{n \times m}.
void matrix_mul(gf *a, gf *b, gf *c, int n, int k, int m) {
    int row;

    for (row = 0; row < n; row++) {
        int col;
        for (col = 0; col < m; col++) {
            gf *pa = a + row * k;
            gf *pb = b + col;
            gf acc = 0;

            int i;
            for (i = 0; i < k; i++, pa++, pb += m)
                acc = GF_ADD(acc, GF_MUL(*pa, *pb));

            c[row * m + col] = acc;
        }
    }
}

// Computes the inverse of a matrix.
// Computes the inverse of a into a using Gauss-Jordan elimination.
// Transform the matrix using pivoting and Gauss elimination to bring it into the form of the identity matrix,
// and apply the transformations to the identity matrix.
// Returns 0 on error, 1 on success
int matrix_inv(gf *a, int k) {
    // Bookkeeping on the pivoting.
    int indxc[k];
    int indxr[k];

    // id_row is used to compare the pivot row to the corresponding identity row in order to speed up computation.
    gf id_row[k];

    // ipiv marks elements already used as pivots.
    int ipiv[k];
    
    // Initialize id_row and ipiv.
    int i;
    for (i = 0; i < k; i++) {
        id_row[i] = 0;
        ipiv[i] = 0;
    }

    int col;
    for (col = 0; col < k; col++) {
        // Look for a non-zero element to use as pivot.
        int irow = -1, icol = -1;

        // First check the diagonal.
        if ((ipiv[col] != 1) && (a[col * k + col] != 0)) {
            irow = col;
            icol = col;
        }
        else {
            // Then search the matrix.
            int row;
            for (row = 0; row < k; row++) {
                if (ipiv[row] != 1) {
                    for (i = 0; i < k; i++) {
                        if (ipiv[i] == 0) {
                            if (a[row * k + i] != 0) {
                                irow = row;
                                icol = i;
                                goto found_pivot;
                            }
                        }
                        else if (ipiv[i] > 1) {
                            fprintf(stderr, "Singular matrix\n");
                            return 0;
                        }
                    }
                }
            }
            fprintf(stderr, "Pivot not found\n");
            return 0;
        }

        found_pivot:
        // Now we got a pivot element in icol and irow.
        ++(ipiv[icol]);

        // Swap rows so the pivot is on the diagonal.
        if (irow != icol) {
            gf tmp;
            for (i = 0; i < k; i++) {
                tmp = a[irow * k + i];
                a[irow * k + i] = a[icol * k + i];
                a[icol * k + i] = tmp;
            }
        }

        // Remember the pivot position.
        indxr[col] = irow;
        indxc[col] = icol;
        gf *pivot_row = a + icol * k;

        // Divide pivot row with the pivot element.
        gf c = pivot_row[icol];
        if (c == 0) {
            fprintf(stderr, "Singular matrix\n");
            return 0;
        }
        else if (c != 1) {
            c = GF_INV(c);
            pivot_row[icol] = 1;
            for (i = 0; i < k; i++)
                pivot_row[i] = GF_MUL(c, pivot_row[i]);
        }

        // Reduce rows. If the pivot row is the identity row, we don't need to subtract the pivot row
        id_row[icol] = 1;
        if (bcmp(pivot_row, id_row, k * sizeof(gf)) != 0) {
            gf *p;
            for (p = a, i = 0; i < k; i++, p += k) {
                // Don't reduce the pivot row.
                if (i != icol) {
                    gf c = p[icol];
                    // Zero out the element corresponding to the pivot element and subtract the pivot row multiplied by the zeroed out element.
                    p[icol] = 0;
                    gf_add_mul(p, pivot_row, c, k);
                }
            }
        }
        id_row[icol] = 0;
    }

    // Descramble the solution.
    for (col = k - 1; col >= 0; col--) {
        if (indxr[col] != indxc[col]) {
            int row;
            gf tmp;
            for (row = 0; row < k; row++) {
                tmp = a[row * k + indxr[col]];
                a[row * k + indxr[col]] = a[row * k + indxc[col]];
                a[row * k + indxc[col]] = tmp;
            }
        }
    }

    return 1;
}

// Computes the inverse of a Vandermonde matrix.
int matrix_inv_vandermonde(gf *a, int k) {
    // Check for a degenerate case.
    if (k == 1)
        return 0;

    // p holds the matrix coefficients x_i.
    gf p[k];
    // c holds the coefficient of P(x) = \prod_{i=0}^{k-1} (x - p_i)
    gf c[k];
    int i, j;
    for (i = 0, j = 1; i < k; i++, j += k) {
        c[i] = 0;
        p[i] = a[j];
    }

    // Construct coefficients. We know c[k] = 1 implicitly. Start with P_0 = x - x_0. We are in 2^m, so x_0 = - x_0.
    c[k - 1] = p[0];
    for (i = 1; i < k; i++) {
        gf p_i = p[i];

        // At each step P_i = x * P_{i - 1} - p_i * P_{i - 1}, so c[j] = c[j] + p[i] * c[j+1], c[k] = 1 (implicit), and c[k-1] = c[k-1] + p_i.
        for (j = k - 1 - i; j < k - 1; j++)
            c[j] = GF_ADD(c[j], GF_MUL(p_i, c[j+1]));
        c[k - 1] = GF_ADD(c[k - 1], p_i);
    }

    // b holds the coefficient for the matrix inversion.
    gf b[k];

    // Do the synthetic division.
    int row;
    for (row = 0; row < k; row++) {
        gf x = p[row];
        gf t = 1;

        b[k - 1] = 1; // c[k]
        for (i = k - 2; i >= 0; i--) {
            b[i] = GF_ADD(c[i + 1], GF_MUL(x, b[i+1]));
            t = GF_ADD(GF_MUL(x, t), b[i]);
        }

        int col;
        for (col = 0; col < k; col++)
            a[col * k + row] = GF_MUL(GF_INV(t), b[col]);
    }

    return 1;
}

#ifdef MATRIX_TEST
void testit(char *name, unsigned int result, unsigned int should) {
    if (result == should) {
        printf("Test %s was successful\n", name);
    }
    else {
        printf("Test %s was not successful, %u should have been %u\n", name, result, should);
    }
}

int main(void) {
    gf matrix1[4 * 4] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    gf matrix2[4 * 4];
    gf matrix3[4 * 4] = { 1, 5, 3, 18, 5, 6, 19, 21, 9, 0, 0, 7, 4, 5, 4, 83 };
    // from mathematica
    gf matrix4[4 * 4] = { 148, 39, 173, 174, 55, 134, 87, 159, 170, 142, 46, 94, 161, 105, 80, 239 };
    gf matrix5[4 * 4] = { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 };

    gf_init();

    gf vand1[4 * 4] = { 1, 2, GF_MUL(2, 2), GF_MUL(2, GF_MUL(2, 2)), 1, 3, GF_MUL(3, 3), GF_MUL(3, GF_MUL(3, 3)), 1, 5, GF_MUL(5, 5), GF_MUL(5, GF_MUL(5, 5)),
            1, 7, GF_MUL(7, 7), GF_MUL(7, GF_MUL(7, 7)) };
    gf vand2[4 * 4];

    testit("invert singular matrix", matrix_inv(matrix5, 4), 0);

    memcpy(matrix2, matrix1, sizeof(matrix1));
    testit("invert matrix", matrix_inv(matrix2, 4), 1);

    int i;
    for (i = 0; i < 16; i++)
        testit("invert matrix", matrix2[i], matrix1[i]);
    testit("invert matrix", matrix_inv(matrix3, 4), 1);
    for (i = 0; i < 16; i++)
        testit("invert matrix", matrix3[i], matrix4[i]);

    memcpy(vand2, vand1, sizeof(vand1));
    testit("vandermonde invert matrix", matrix_inv_vandermonde(vand1, 4), 1);
    testit("invert matrix", matrix_inv(vand2, 4), 1);
    for (i = 0; i < 16; i++)
        testit("vandermonde invert matrix", vand1[i], vand2[i]);

    return 0;
}

#endif /* MATRIX_TEST */
