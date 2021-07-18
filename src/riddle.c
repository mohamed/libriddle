#include <stdlib.h>
#include <stdint.h>

#include <openssl/rand.h>

#include "riddle.h"

/*
 * This function generates a list of unique random numbers
 * The list has list_length elements where each element has size bytes
 */
static int32_t gen_unique_random_list(
    const uint32_t list_length,
    const size_t size,
    BIGNUM ** list)
{
    uint32_t i = 0, j = 0;
    int32_t is_unique = 0, rv = 0, exitcode = -1;

    if (NULL == list || list_length < 2 || size < 1) {
        return -1;
    }

    for (i = 0; i < list_length; i++) {
        is_unique = 0;
        while (!is_unique) {
spin:
            list[i] = BN_new();
            rv = BN_rand(list[i], size, -1, 1);
            if (1 != rv) goto gen_cleanup;
            rv = BN_add(list[i], BN_value_one(), list[i]);
            if (1 != rv) goto gen_cleanup;
            for (j = 0; j < i; j++) {
                if (0 == BN_cmp(list[i], list[j])) {
                    goto spin;
                }
            }
            is_unique = 1;
        }
    }
    exitcode = 0;
gen_cleanup:
    return exitcode;
}

int32_t riddle_split(
    const BIGNUM * prime,
    const uint32_t num_shares,
    const uint32_t threshold,
    const BIGNUM * secret,
    struct riddle_share * shares)
{
    uint32_t i = 0, j = 0;
    size_t share_size = 0;
    int32_t exitcode = 0, rv = 0;
    BIGNUM ** coefficients = NULL, ** xs = NULL;
    BIGNUM * y, * tmp, * degree;

    /* Check the inputs */
    if (NULL == prime ||
        NULL == secret ||
        BN_cmp(secret, prime) >= 0 ||
        NULL == shares ||
        threshold < 2 ||
        threshold > num_shares ||
        num_shares < 2) {
        return -1;
    }

    coefficients = (BIGNUM **) calloc(threshold - 1,  sizeof(BIGNUM *));
    if (NULL == coefficients) {
        return -1;
    }
    xs = (BIGNUM **) calloc(num_shares, sizeof(BIGNUM *));
    if (NULL == xs) {
        free(coefficients);
        coefficients = NULL;
        return -1;
    }

    /* Initialize coefficients and xs */
    share_size = BN_num_bits(prime) - 1;
    rv = gen_unique_random_list(threshold-1, share_size, coefficients);
    rv += gen_unique_random_list(num_shares, share_size, xs);
    if (0 != rv) {
        exitcode = -1;
        goto split_cleanup;
    }

    BN_CTX * ctx = BN_CTX_new();
    if (NULL == ctx) { abort(); }
    tmp = BN_new();
    if (NULL == tmp) { abort(); }
    for (i = 0; i < num_shares; i++) {
        BN_copy(shares[i].x, xs[i]);
        y = BN_dup(secret);
        if (NULL == y) { abort(); }
        degree = BN_dup(BN_value_one());
        if (NULL == degree) { abort(); }
        for (j = 0; j < (threshold - 1); j++) {
            rv = BN_mod_exp(tmp, shares[i].x, degree, prime, ctx);
            if (1 != rv) { exitcode = -1; goto handle_err; }
            rv = BN_mod_mul(tmp, coefficients[j], tmp, prime, ctx);
            if (1 != rv) { exitcode = -1; goto handle_err; }
            rv = BN_mod_add(y, tmp, y, prime, ctx);
            if (1 != rv) { exitcode = -1; goto handle_err; }
            rv = BN_mod_add(degree, degree, BN_value_one(), prime, ctx);
            if (1 != rv) { exitcode = -1; goto handle_err; }
        }
        BN_clear_free(degree);
        BN_copy(shares[i].y, y);
        BN_clear_free(y);
        /* TODO: Can this if happen? */
        if (0 == BN_cmp(shares[i].x, secret) ||
              0 == BN_cmp(shares[i].y, secret)) {
            exitcode = -3;
            break;
        }
    }
handle_err:
    BN_clear_free(tmp);
    BN_CTX_free(ctx);

split_cleanup:
    if (0 != exitcode) {
        for (i = 0; i < num_shares; i++) {
            BN_zero(shares[i].x);
            BN_zero(shares[i].y);
        }
    }
    /* Clear data */
    for (i = 0; i < num_shares; i++) {
        BN_clear_free(xs[i]);
    }
    for (i = 0; i < (threshold - 1); i++) {
        BN_clear_free(coefficients[i]);
    }
    free(coefficients);
    coefficients = NULL;
    free(xs);
    xs = NULL;
    return exitcode;
}


int32_t riddle_join(
    const BIGNUM * prime,
    const uint32_t num_shares,
    const struct riddle_share * shares,
    BIGNUM * secret)
{
    uint32_t j = 0, m = 0;
    BIGNUM * product, * d, * r;
    BIGNUM * reconstructed;

    if (NULL == prime ||
        NULL == shares ||
        num_shares < 2 ||
        NULL == secret) {
      return -1;
    }

    for (j = 0; j < num_shares; j++) {
        if (BN_cmp(shares[j].x, prime) >= 0 ||
              BN_cmp(shares[j].y, prime) >= 0) {
            return -1;
        }
    }

    BN_CTX * ctx = BN_CTX_new();
    if (NULL == ctx) { abort(); }
    reconstructed = BN_new();
    if (NULL == reconstructed) { abort(); }
    BN_zero(reconstructed);
    BIGNUM * p = NULL;

    for (j = 0; j < num_shares; j++) {
        product = BN_new();
        if (NULL == product) { abort(); }
        BN_one(product);
        for (m = 0; m < num_shares; m++) {
            d = BN_new();
            r = BN_new();
            if (m != j) {
                BN_sub(d, shares[m].x, shares[j].x);
                p = BN_mod_inverse(d, d, prime, ctx);
                if (NULL == p) {
                    BN_clear_free(d);
                    BN_clear_free(r);
                    BN_clear_free(product);
                    BN_clear_free(reconstructed);
                    BN_CTX_free(ctx);
                    return -1;
                }
                BN_mod_mul(r, shares[m].x, d, prime, ctx);
                BN_mod_mul(product, product, r, prime, ctx);
            }
            BN_clear_free(d);
            BN_clear_free(r);
        }
        BN_mod_mul(product, shares[j].y, product, prime, ctx);
        BN_mod_add(reconstructed, reconstructed, product, prime, ctx);
        BN_clear_free(product);
    }
    BN_copy(secret, reconstructed);
    BN_clear_free(reconstructed);
    BN_CTX_free(ctx);

    return 0;
}

