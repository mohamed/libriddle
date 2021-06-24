#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <gmp.h>
#include <openssl/rand.h>

#include "../src/riddle.h"

#define STRINGFY(x) #x


void riddle_assert(const int x) {
    if (!(x)) {
        fprintf(stderr, "%s\n", STRINGFY(x));
        exit(EXIT_FAILURE);
    }
}

int do_test(const mpz_t prime,
            const uint32_t N,
            const uint32_t L,
            const uint32_t S)
{
    int32_t retval = 0;
    uint32_t i = 0, j = 0;
    mpz_t secret, reconstructed;
    struct riddle_share shares[N];
    char secret_buf[S];

    mpz_init(secret);
    gmp_printf("Prime = %Zd\n", prime);
    gmp_printf("Prime size = %zu\n", mpz_sizeinbase(prime, 2));

    for (i = 0; i < N; i++) {
        mpz_init(shares[i].x);
        mpz_init(shares[i].y);
    }

    for (j = 0; j < L; j++) {
        retval = RAND_bytes((unsigned char *)secret_buf, S);
        if (1 != retval) abort();

        mpz_import (secret, sizeof(secret_buf)/sizeof(secret_buf[0]), 1,
                    sizeof(secret_buf[0]), 0, 0, secret_buf);
        riddle_assert(mpz_sizeinbase(secret, 2) < mpz_sizeinbase(prime, 2));

        retval = riddle_split(prime, N, 4, secret, shares);
        riddle_assert(retval == 0);
        riddle_print_shares(shares, N);

        for (i = 2; i < N; i++) {
            mpz_init(reconstructed);
            retval = riddle_join(prime, i,
                                (const struct riddle_share *) shares,
                                 reconstructed);
            if (i >= 4) {
                riddle_assert(retval == 0 && \
                              mpz_cmp(reconstructed, secret) == 0);
            } else {
                riddle_assert(retval == 0 && \
                              mpz_cmp(reconstructed, secret) != 0);
            }
            mpz_clear(reconstructed);
        }
    }

    for (i = 0; i < N; i++) {
        mpz_clear(shares[i].x);
        mpz_clear(shares[i].y);
    }
    mpz_clear(secret);
    return EXIT_SUCCESS;
}
