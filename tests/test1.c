#include "common.h"

#define N (5)
#define L (3)
#define S (2)

static char prime_str[] = "1000008000001";

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
