#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <gmp.h>

void riddle_assert(const int x);
int do_test(const mpz_t prime,
            const uint32_t N,
            const uint32_t L,
            const uint32_t S);

#endif
