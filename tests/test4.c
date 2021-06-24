#include "common.h"

#define N     (5)
#define S     (512/8)
#define L     (3)

int main()
{
    int exitcode = EXIT_FAILURE;
    mpz_t prime, thirtyfour;

    mpz_init(prime);
    mpz_init(thirtyfour);

    /*
     * http://primes.utm.edu/curios/page.php?number_id=4556
     * Prime = (1597 * 34 ^ 2606) + 1
     */
    mpz_set_ui(thirtyfour, 34);
    mpz_pow_ui(prime, thirtyfour, 2606);
    mpz_mul_ui(prime, prime, 1597);
    mpz_add_ui(prime, prime, 1);
    mpz_clear(thirtyfour);

    exitcode = do_test(prime, N, L, S);
    mpz_clear(prime);

    return exitcode;
}
