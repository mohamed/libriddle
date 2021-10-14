#ifndef RIDDLE_H_
#define RIDDLE_H_

#include <stdint.h>

#include <openssl/bn.h>

/**
 * Represent a share with its X-Y coordinates
 */
struct riddle_share {
  BIGNUM * x;
  BIGNUM * y;
};


/**
 * @brief Splits the given \a secret into \a num_shares shares
 *
 * This function implements the \a secret splitting mechanism defined in:
 * Adi Shamir, "How to share a secret", Communications of the ACM 22(11)
 * pages 612-613 doi:10.1145/359168.359176
 *
 * The secret is split into \a num_shares that are stored as pairs of
 * (x,y) in the given \a shares. All the numbers must be less than the
 * given \a prime
 *
 * @param prime A prime number used to construct a Galois field
 * @param num_shares The number of generated shares
 * @param threshold The minimum number of shares needed to reconstruct secret
 * @param secret The secret to be split
 * @param shares The list of the shares
 * @return 0 if successful, -1 otherwise.
 */
int32_t riddle_split(
    const BIGNUM * prime,
    const uint32_t num_shares,
    const uint32_t threshold,
    const BIGNUM * secret,
    struct riddle_share * shares);

/**
 * @brief Reconstruct a secret from the given shares
 *
 * This function reconstruct the \a secret from the given \a shares_xs
 * and \a shares_ys according to Shamir's algorithm.
 *
 * @param prime A prime number used to construct a Galois field
 * @param num_shares The number of shares
 * @param shares The list of the shares
 * @param secret The reconstructed secret
 * @return 0 if successful, -1 otherwise
 */
int32_t riddle_join(
    const BIGNUM * prime,
    const uint32_t num_shares,
    const struct riddle_share * shares,
    BIGNUM * secret);


#endif
