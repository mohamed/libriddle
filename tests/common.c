#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <openssl/rand.h>

#include "common.h"


#define STRINGFY(x) #x


void riddle_print_shares(const struct riddle_share * shares,
                         const uint32_t length)
{
    uint32_t i = 0;
    char * x, * y;

    if (NULL == shares || 0 == length) return;

    printf("==========================================================\n");
    printf("                       List of Shares                     \n");
    printf("==========================================================\n");
    for (i = 0; i < length; i++) {
        printf("Share %d:\n", i + 1);
        x = BN_bn2hex(shares[i].x);
        y = BN_bn2hex(shares[i].y);
        printf("\tPart 1: %s\n", x);
        printf("\tPart 2: %s\n", y);
        printf("\n");
        OPENSSL_free(x);
        OPENSSL_free(y);
        x = NULL;
        y = NULL;
    }
}

void riddle_assert(const int x) {
    if (!(x)) {
        fprintf(stderr, "%s\n", STRINGFY(x));
        exit(EXIT_FAILURE);
    }
}

int do_test(const BIGNUM * prime,
            const uint32_t N,
            const uint32_t L,
            const uint32_t S,
            const uint32_t check_prime)
{
    int32_t retval = 0;
    uint32_t i = 0;
    BIGNUM * secret, * reconstructed;
    struct riddle_share * shares = NULL;
    char * str = NULL;
    BN_CTX * ctx = NULL;

    if (NULL == prime || 0 == N) {
      return EXIT_FAILURE;
    }
    shares = (struct riddle_share *) calloc(N, sizeof(struct riddle_share));
    if (NULL == shares) {
      return EXIT_FAILURE;
    }

    if (check_prime) {
      ctx = BN_CTX_new();
      retval = BN_is_prime_ex(prime, BN_prime_checks, ctx, NULL);
      if (1 != retval) { abort(); }
      BN_CTX_free(ctx);
    }

    secret = BN_new();
    str = BN_bn2hex(prime);
    printf("Prime = %s\n", str);
    OPENSSL_free(str);
    printf("Prime size = %d\n", BN_num_bits(prime));

    for (i = 0; i < N; i++) {
        shares[i].x = BN_new();
        shares[i].y = BN_new();
    }

    retval = BN_rand(secret, (int) (S * 8), -1, 0);
    if (1 != retval) abort();
    str = BN_bn2hex(secret);
    printf("Secret = %s\n", str);
    OPENSSL_free(str);
    riddle_assert(BN_num_bits(secret) < BN_num_bits(prime));

    retval = riddle_split(prime, N, L, secret, shares);
    riddle_assert(retval == 0);
    riddle_print_shares(shares, N);

    for (i = 2; i < N; i++) {
        reconstructed = BN_new();
        retval = riddle_join(prime, i,
                            (const struct riddle_share *) shares,
                             reconstructed);
        if (i >= L) {
            riddle_assert(retval == 0);
            riddle_assert(BN_cmp(reconstructed, secret) == 0);
        } else {
            riddle_assert(retval == 0);
            riddle_assert(BN_cmp(reconstructed, secret) != 0);
        }
        BN_clear_free(reconstructed);
    }

    for (i = 0; i < N; i++) {
        BN_clear_free(shares[i].x);
        BN_clear_free(shares[i].y);
    }
    str = NULL;
    BN_clear_free(secret);
    free(shares);
    shares = NULL;

    return EXIT_SUCCESS;
}
