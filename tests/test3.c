#include "common.h"

#define N   (9)
#define S   (512/8)
#define L   (5)

int main()
{
    int exitcode = EXIT_FAILURE;
    BIGNUM * prime, * tmp, * ten, * nine;
    BN_CTX * ctx = NULL;
    prime = tmp = ten = nine = NULL;

    prime = BN_new();
    tmp = BN_new();
    ten = BN_new();
    nine = BN_new();

    /* 1000 digit prime (3322 bits)
     * Source: http://primes.utm.edu/curios/page.php?number_id=4032
     * Prime = (85 * (10 ^ 999) + 41) / 9 + 4040054550
     */
    ctx = BN_CTX_new();
    BN_set_word(ten, 10);
    BN_set_word(nine, 999);
    BN_exp(prime, ten, nine, ctx);
    BN_set_word(nine, 85);
    BN_mul(prime, prime, nine, ctx);
    BN_set_word(nine, 41);
    BN_add(prime, prime, nine);
    BN_set_word(nine, 9);
    BN_div(tmp, NULL, prime, nine, ctx);
    BN_set_word(nine, 4040054550UL);
    BN_add(prime, tmp, nine);

    exitcode = do_test(prime, N, L, S, 0);
    BN_CTX_free(ctx);
    BN_free(prime);
    BN_free(ten);
    BN_free(nine);
    BN_free(tmp);

    return exitcode;
}
