#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stdlib.h>

#include "../src/riddle.h"


void riddle_print_shares(const struct riddle_share * shares,
                         const uint32_t length);
void riddle_assert(const int x);
int do_test(const BIGNUM * prime,
            const uint32_t N,
            const uint32_t L,
            const uint32_t S,
            const uint32_t check_prime);

#endif
