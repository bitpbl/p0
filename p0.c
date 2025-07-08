/*
 * p0: a 512-bit permutation (C99, scalar)
 *  targeting <7cpb at 12 rounds
 *  8 × 64-bit words state
 *
 * structure per round:
 *  inject RC
 *  4 parallel ARX-boxes on word-pairs with round-varying schedule
 *  intra-word byte-wise MDS diffusion
 *  lightweight global mixing (on purpose, to keep cycle counts low)
 *
 * note: this pushed out 6.6cpb @ 12 rounds on my alder lake-h cpu
 *
 * this permutation is for the zero cryptographic suite, stylized p (subscript) 0 and pronounced point zero
 * by pbl <3
 */

#include <stdint.h>
#include <stddef.h>

#define MAX_ROUNDS 16
#define PERM_ROUNDS 4

/* rotation schedules, see rotations.py */
static const unsigned ROT1[MAX_ROUNDS] = {
    49,  1,  9, 13, 41, 59, 21, 23,
    39,  5, 45,  3, 51, 63, 11, 57
};
static const unsigned ROT2[MAX_ROUNDS] = {
    43, 11, 41, 25, 51, 47, 35,  3,
    21, 63,  9, 45, 49, 17, 55, 19
};
static const unsigned ROT3[MAX_ROUNDS] = {
    49, 63, 39, 61, 45, 37, 27, 41,
    53, 21, 59, 55, 35, 33, 19, 29
};
static const unsigned ROT4[MAX_ROUNDS] = {
    11, 45, 59, 41, 33, 17, 49, 39,
    15,  5, 27, 51, 57, 53, 47,  1
};

/* round constants, see constants.py */
static const uint64_t RC[MAX_ROUNDS] = {
    0x313c6532d658201aULL, 0x956d549917eac3c0ULL,
    0x083d4edaf496ee20ULL, 0x0cb386eeab6965d2ULL,
    0x6f856ac36b7642e0ULL, 0xe81887430eb2f988ULL,
    0xaed89bd9076d6e8bULL, 0xabe63903dd74dfbbULL,
    0x1add4c445f6c7432ULL, 0xd1d113667136bbccULL,
    0xcbf0b33854f1a287ULL, 0x37e6e010269e7e78ULL,
    0x9856a37486c20a3cULL, 0x5020a4e865c9be36ULL,
    0x1401db7b7dac82b8ULL, 0x62cdab713371b069ULL
};

/* left rotate a 64-bit word */
static inline uint64_t rotl64(uint64_t x, unsigned r) {
    return (x << (r & 63)) | (x >> ((64 - r) & 63));
}

__attribute__((noinline)) void p0(uint64_t state[8]) {
    for (size_t r = 0; r < PERM_ROUNDS; r++) {
        /* round constant injection */
        state[(r * 3) & 7] ^= RC[r];

        /* nonlinear ARX layer */
        for (int i = 0; i < 4; i++) {
            int a = (2 * i + r) & 7;
            int b = (a + 1) & 7;

            uint64_t x = state[a];
            uint64_t y = state[b];

            x += rotl64(y, ROT1[r]);
            y ^= rotl64(x, ROT2[r]);
            x += rotl64(y, ROT3[r]);
            y ^= rotl64(x, ROT4[r]);

            state[a] = x;
            state[b] = y;
        }

        /* intra-word MDS diffusion */
        for (int i = 0; i < 8; i++) {
            uint64_t w = state[i];
            state[i] = w
                ^ rotl64(w, (3 * r + 7) & 63)
                ^ rotl64(w, (5 * r + 23) & 63)
                ^ rotl64(w, (7 * r + 41) & 63);
        }

        /* global lightweight mixing */
        uint64_t tmp[8];
        int offset1 = (r * 2 + 1) & 7;
        int offset2 = (r * 3 + 3) & 7;

        for (int i = 0; i < 8; i++) {
            tmp[i] = state[i]
                ^ state[(i + offset1) & 7]
                ^ state[(i + offset2) & 7];
        }
        for (int i = 0; i < 8; i++) {
            state[i] = tmp[i];
        }
    }
}
