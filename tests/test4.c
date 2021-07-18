#include "common.h"

#define N     (5)
#define S     (512/8)
#define L     (3)

int main()
{
    int exitcode = EXIT_FAILURE;
    BIGNUM * prime, * tmp1, * tmp2;

    BN_CTX * ctx = BN_CTX_new();
    prime = BN_new();
    tmp1 = BN_new();
    tmp2 = BN_new();

    /*
     * http://primes.utm.edu/curios/page.php?number_id=4556
     * Prime = (1597 * 34 ^ 2606) + 1
     */
    BN_set_word(tmp1, 34);
    BN_set_word(tmp2, 2606);
    BN_exp(prime, tmp1, tmp2, ctx);
    BN_set_word(tmp1, 1597);
    BN_mul(prime, prime, tmp1, ctx);
    BN_add(prime, prime, BN_value_one());

    exitcode = do_test(prime, N, L, S, 0);
    BN_free(prime);
    BN_free(tmp1);
    BN_free(tmp2);
    BN_CTX_free(ctx);

    return exitcode;
}
