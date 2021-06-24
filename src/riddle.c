#include <stdlib.h>
#include <stdint.h>

#include <openssl/rand.h>

#include "riddle.h"


static int32_t gen_unique_random_list(
    const uint32_t list_length,
    const size_t size,
    mpz_t * list)
{
    uint32_t i = 0, j = 0;
    int32_t is_unique = 0, rv = 0, exitcode = -1;
    uint8_t * buf = NULL;

    if (!list || list_length < 2 || size < 1) {
        return -1;
    }

    buf = (uint8_t *) calloc(size, sizeof(uint8_t));
    if (!buf) return -1;

    for (i = 0; i < list_length; i++) {
        mpz_init(list[i]);
        is_unique = 0;
        while (!is_unique) {
spin:
            rv = RAND_bytes(buf, (int) size);
            if (1 != rv) goto gen_cleanup;
            mpz_import(list[i], size, 1, sizeof(uint8_t), 0, 0, buf);
            mpz_add_ui(list[i], list[i], 1);
            for (j = 0; j < i; j++) {
                if (0 == mpz_cmp(list[i], list[j])) {
                    goto spin;
                }
            }
            is_unique = 1;
        }
    }
    exitcode = 0;
gen_cleanup:
    free(buf);
    buf = NULL;
    return exitcode;
}

int32_t riddle_split(
    const mpz_t prime,
    const uint32_t num_shares,
    const uint32_t threshold,
    const mpz_t secret,
    struct riddle_share * shares)
{
    uint32_t i = 0, j = 0;
    size_t share_size_in_bytes = 0;
    int32_t exitcode = 0, rv = 0;
    mpz_t * coefficients = NULL, * xs = NULL;
    mpz_t y, tmp, degree;

    /* Check the inputs */
    if (mpz_cmp(secret, prime) >= 0 ||
        !shares ||
        threshold < 2 ||
        threshold > num_shares ||
        num_shares < 2) {
        return -1;
    }

    coefficients = (mpz_t *) calloc(threshold - 1,  sizeof(mpz_t));
    if (!coefficients) {
        return -1;
    }
    xs = (mpz_t *) calloc(num_shares, sizeof(mpz_t));
    if (!xs) {
        free(coefficients);
        coefficients = NULL;
        return -1;
    }

    /* Initialize coefficients and xs */
    share_size_in_bytes = (mpz_sizeinbase(prime, 2) - 1) / 8;
    rv = gen_unique_random_list(threshold-1,
          share_size_in_bytes, coefficients);
    rv += gen_unique_random_list(num_shares, share_size_in_bytes, xs);
    if (0 != rv) {
        exitcode = -1;
        goto split_cleanup;
    }

    mpz_init(tmp);
    for (i = 0; i < num_shares; i++) {
        mpz_set(shares[i].x, xs[i]);
        mpz_init_set(y, secret);
        mpz_init_set_ui(degree, 1);
        for (j = 0; j < (threshold - 1); j++) {
            mpz_powm_sec(tmp, shares[i].x, degree, prime);
            mpz_addmul(y, coefficients[j], tmp);
            mpz_add_ui(degree, degree, 1);
        }
        mpz_clear(degree);
        mpz_mod(y, y, prime);
        mpz_set(shares[i].y, y);
        mpz_clear(y);
        if (0 == mpz_cmp(shares[i].x, secret) ||
              0 == mpz_cmp(shares[i].y, secret)) {
            exitcode = -1;
            break;
        }
    }
    mpz_clear(tmp);

split_cleanup:
    if (0 != exitcode) {
        for (i = 0; i < num_shares; i++) {
            mpz_set_ui(shares[i].x, 0);
            mpz_set_ui(shares[i].y, 0);
        }
    }
    /* Clear data */
    for (i = 0; i < num_shares; i++) {
        mpz_set_ui(xs[i], 0);
        mpz_clear(xs[i]);
    }
    for (i = 0; i < (threshold - 1); i++) {
        mpz_set_ui(coefficients[i], 0);
        mpz_clear(coefficients[i]);
    }
    free(coefficients);
    coefficients = NULL;
    free(xs);
    xs = NULL;
    return exitcode;
}


int32_t riddle_join(
    const mpz_t prime,
    const uint32_t num_shares,
    const struct riddle_share * shares,
    mpz_t secret)
{
    uint32_t j = 0, m = 0;
    int32_t retval = -1;
    mpz_t product, d, r;
    mpz_t reconstructed;

    if (!shares || num_shares < 2) {
      return -1;
    }

    for (j = 0; j < num_shares; j++) {
        if (mpz_cmp(shares[j].x, prime) >= 0 ||
              mpz_cmp(shares[j].y, prime) >= 0) {
            return -1;
        }
    }

    mpz_init_set_ui(reconstructed, 0);

    for (j = 0; j < num_shares; j++) {
        mpz_init_set_ui(product, 1);
        for (m = 0; m < num_shares; m++) {
            mpz_init(d);
            mpz_init(r);
            if (m != j) {
                mpz_sub(d, shares[m].x, shares[j].x);
                retval = mpz_invert(d, d, prime);
                if (0 == retval) {
                    mpz_clear(d);
                    mpz_clear(r);
                    mpz_clear(product);
                    mpz_clear(reconstructed);
                    return -1;
                }
                mpz_mul(r, shares[m].x, d);
                mpz_mul(product, product, r);
            }
            mpz_clear(d);
            mpz_clear(r);
        }
        mpz_addmul(reconstructed, shares[j].y, product);
        mpz_mod(reconstructed, reconstructed, prime);
        mpz_clear(product);
    }
    mpz_set(secret, reconstructed);
    mpz_set_ui(reconstructed, 0);
    mpz_clear(reconstructed);

    return 0;
}

void riddle_print_shares(
    const struct riddle_share * shares,
    const uint32_t length)
{
    uint32_t i = 0;

    if (!shares || !length) return;

    gmp_printf("==========================================================\n");
    gmp_printf("                       List of Shares                     \n");
    gmp_printf("==========================================================\n");
    for (i = 0; i < length; i++) {
        gmp_printf("Share %d:\n", i + 1);
        gmp_printf("\tPart 1: %Zx\n", shares[i].x);
        gmp_printf("\tPart 2: %Zx\n", shares[i].y);
        gmp_printf("\n");
    }
}
