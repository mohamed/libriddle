#include <stdlib.h>
#include <stdint.h>

#include <openssl/rand.h>

#include "riddle.h"

/*
 * This function generates a list of unique random numbers
 * The list has list_length elements where each element has size bytes
 */
static int32_t gen_unique_randoms(
    const uint32_t list_length,
    const size_t size,
    BIGNUM ** list)
{
    uint32_t i = 0, j = 0;
    int32_t is_unique, rv, exitcode;

    if (NULL == list || list_length < 2 || size < 1) {
        exitcode = -1;
    } else {
        exitcode = 0;
        for (i = 0; i < list_length; i++) {
            is_unique = 0;
            while (!is_unique) {
                list[i] = BN_new();
                if (NULL == list[i]) {
                  exitcode = -1;
                  break;
                }
                rv = BN_rand(list[i], (int) size, -1, 1);
                if (1 != rv) {
                    exitcode = -1;
                    break;
                }
                rv = BN_add(list[i], BN_value_one(), list[i]);
                if (1 != rv) {
                    exitcode = -1;
                    break;
                }
                is_unique = 1;
                for (j = 0; j < i; j++) {
                    if (0 == BN_cmp(list[i], list[j])) {
                        is_unique = 0;
                        break;
                    }
                }
                if (!is_unique) {
                    BN_clear_free(list[i]);
                }
            }
            if (0 != exitcode) {
                /* TODO: clear all the previous ones */
                BN_clear_free(list[i]);
                break;
            }
        }
    }
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
    BIGNUM ** coeffs = NULL, ** xs = NULL;
    BIGNUM * y = NULL, * tmp = NULL, * degree = NULL;
    BN_CTX * ctx = NULL;

    /* Check the inputs */
    if ((NULL == prime) ||
        (NULL == secret) ||
        (BN_cmp(secret, prime) >= 0) ||
        (NULL == shares) ||
        (threshold < 2) ||
        (threshold > num_shares) ||
        (num_shares < 2)) {
        goto cleanup_s2;
    }

    coeffs = (BIGNUM **) calloc(threshold - 1,  sizeof(BIGNUM *));
    if (NULL == coeffs) {
        goto cleanup_s2;
    }
    xs = (BIGNUM **) calloc(num_shares, sizeof(BIGNUM *));
    if (NULL == xs) {
        free(coeffs);
        coeffs = NULL;
        goto cleanup_s2;
    }

    /* Initialize coefficients and xs */
    share_size = (size_t) BN_num_bits(prime) - 1;
    rv = gen_unique_randoms(threshold-1, share_size, coeffs);
    rv += gen_unique_randoms(num_shares, share_size, xs);
    if (0 != rv) {
        exitcode = -1;
        goto cleanup_s;
    }

    exitcode = -1;
    ctx = BN_CTX_new();
    if (NULL == ctx) { goto cleanup_s; }
    tmp = BN_new();
    if (NULL == tmp) { goto cleanup_s; }
    for (i = 0; i < num_shares; i++) {
        BN_copy(shares[i].x, xs[i]);
        y = BN_dup(secret);
        if (NULL == y) { goto cleanup_s; }
        degree = BN_dup(BN_value_one());
        if (NULL == degree) { goto cleanup_s; }
        for (j = 0; j < (threshold - 1); j++) {
            rv = BN_mod_exp(tmp, shares[i].x, degree, prime, ctx);
            if (1 != rv) { goto cleanup_s; }
            rv = BN_mod_mul(tmp, coeffs[j], tmp, prime, ctx);
            if (1 != rv) { goto cleanup_s; }
            rv = BN_mod_add(y, tmp, y, prime, ctx);
            if (1 != rv) { goto cleanup_s; }
            rv = BN_mod_add(degree, degree, BN_value_one(), prime, ctx);
            if (1 != rv) { goto cleanup_s; }
        }
        BN_clear_free(degree);
        degree = NULL;
        BN_copy(shares[i].y, y);
        BN_clear_free(y);
        y = NULL;
        /* TODO: Can this if happen? */
        if (0 == BN_cmp(shares[i].x, secret) ||
              0 == BN_cmp(shares[i].y, secret)) {
            goto cleanup_s;
        }
    }
    exitcode = 0;
cleanup_s:
    BN_clear_free(degree);
    BN_clear_free(y);
    BN_clear_free(tmp);
    BN_CTX_free(ctx);
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
        BN_clear_free(coeffs[i]);
    }
    free(coeffs);
    coeffs = NULL;
    free(xs);
    xs = NULL;
cleanup_s2:
    return exitcode;
}


int32_t riddle_join(
    const BIGNUM * prime,
    const uint32_t num_shares,
    const struct riddle_share * shares,
    BIGNUM * secret)
{
    uint32_t j = 0, m = 0;
    BIGNUM * product = NULL, * d = NULL, * r = NULL;
    BIGNUM * reconstructed = NULL;
    BN_CTX * ctx = NULL;
    BIGNUM * p = NULL;
    int32_t exitcode = -1;
    int rv;

    if ((NULL == prime) ||
        (NULL == shares) ||
        (num_shares < 2) ||
        (NULL == secret)) {
      goto cleanup_j2;
    }

    for (j = 0; j < num_shares; j++) {
        if (BN_cmp(shares[j].x, prime) >= 0 ||
              BN_cmp(shares[j].y, prime) >= 0) {
            goto cleanup_j2;
        }
    }

    ctx = BN_CTX_new();
    if (NULL == ctx) { goto cleanup_j; }
    reconstructed = BN_new();
    if (NULL == reconstructed) { goto cleanup_j; }
    BN_zero(reconstructed);

    exitcode = -1;
    for (j = 0; j < num_shares; j++) {
        product = BN_new();
        if (NULL == product) { goto cleanup_j; }
        BN_one(product);
        for (m = 0; m < num_shares; m++) {
            d = BN_new();
            if (NULL == d) { goto cleanup_j; }
            r = BN_new();
            if (NULL == r) { goto cleanup_j; }
            if (m != j) {
                rv = BN_sub(d, shares[m].x, shares[j].x);
                if (1 != rv) { goto cleanup_j; }
                p = BN_mod_inverse(d, d, prime, ctx);
                if (NULL == p) {
                    BN_clear_free(d);
                    BN_clear_free(r);
                    BN_clear_free(product);
                    BN_clear_free(reconstructed);
                    BN_CTX_free(ctx);
                    goto cleanup_j2;
                }
                rv = BN_mod_mul(r, shares[m].x, d, prime, ctx);
                if (1 != rv) { goto cleanup_j; }
                rv = BN_mod_mul(product, product, r, prime, ctx);
                if (1 != rv) { goto cleanup_j; }
            }
            BN_clear_free(d);
            BN_clear_free(r);
        }
        rv = BN_mod_mul(product, shares[j].y, product, prime, ctx);
        if (1 != rv) { goto cleanup_j; }
        rv = BN_mod_add(reconstructed, reconstructed, product, prime, ctx);
        if (1 != rv) { goto cleanup_j; }
        BN_clear_free(product);
    }
    exitcode = 0;
cleanup_j:
    BN_copy(secret, reconstructed);
    BN_clear_free(reconstructed);
    BN_CTX_free(ctx);
cleanup_j2:
    return exitcode;
}

