#include "common.h"

#define N   (9)
#define S   (512/8)
#define L   (5)

int main()
{
    int exitcode = EXIT_FAILURE;
    mpz_t prime, ten, nine;

    mpz_init(prime);
    mpz_init(ten);
    mpz_init(nine);

    /* 1000 digit prime (3322 bits)
     * Source: http://primes.utm.edu/curios/page.php?number_id=4032
     * Prime = (85 * (10 ^ 999) + 41) / 9 + 4040054550
     */
    mpz_set_ui(ten, 10);
    mpz_set_ui(nine, 9);
    mpz_pow_ui(prime, ten, 999);
    mpz_mul_ui(prime, prime, 85);
    mpz_add_ui(prime, prime, 41);
    mpz_cdiv_q(prime, prime, nine);
    mpz_add_ui(prime, prime, 4040054550UL);
    mpz_clear(ten);
    mpz_clear(nine);

    exitcode = do_test(prime, N, L, S);
    mpz_clear(prime);

    return exitcode;
}
