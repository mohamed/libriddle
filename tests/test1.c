#include "common.h"

#define N (5)
#define L (10)
#define S (2)

static char prime_str[] = "909360333829";

int main()
{
    int exitcode = EXIT_FAILURE;
    mpz_t prime;
    mpz_init_set_str(prime, prime_str, 10);
    exitcode = do_test(prime, N, L, S);
    mpz_clear(prime);

    return exitcode;
}
