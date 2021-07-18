#include "common.h"

#define N    (7)
#define L    (3)
#define S    (512/8)

/*
 * Prime source is:
 * https://primes.utm.edu/lists/small/small3.html
 */
static char prime_str[] = "203956878356401977405765866929034577280193993314348263094772646453283062722701277632936616063144088173312372882677123879538709400158306567338328279154499698366071906766440037074217117805690872792848149112022286332144876183376326512083574821647933992961249917319836219304274280243803104015000563790123";

int main()
{
    int exitcode = EXIT_FAILURE;
    BIGNUM * prime = NULL;
    int rv = 0;
    rv = BN_dec2bn(&prime, prime_str);
    if (0 == rv) abort();
    exitcode = do_test(prime, N, L, S, 1);
    BN_free(prime);

    return exitcode;
}
