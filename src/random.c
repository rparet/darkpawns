/* ************************************************************************
*   File: random.c                                      Part of CircleMUD *
*  Usage: pseudo-random number generator                                  *
************************************************************************ */

/*
 * This file implements a high quality psuedo-random number generator.
 *
 * It uses a complimentary-multiply-with-carry RNG with a period of
 * 2^131104 as described by Marsaglia.
 */

#include <stdint.h>
#include "random.h"

static void cmwc_seed(uint32_t seed);
static uint32_t cmwc_next();

/* Generic entry point to the RNG's seeding function. */
void prng_seed(uint32_t seed)
{
  cmwc_seed(seed);
}

/* Get the next 32-bit integer in the random sequence. */
uint32_t prng_next()
{
  return cmwc_next();
}

/* Returns a float from a uniform distribution over [0,1). */
float prng_uniform()
{
  const float f = 2.328306437e-10f; /* 1.0 / (2^32 - 1) */
  return prng_next() * f;
}

/* CMWC PRNG implementation */

/* 1024 seed values */
static uint32_t Q[1024];

/* Seeds the cmwc RNG using xorshift. */
static void cmwc_seed(uint32_t j)
{
  int i;

  for (i = 0; i < 1024; i++) {
    j ^= j << 13;
    j ^= j >> 17;
    j ^= j << 5;
    Q[i] = j;
  }
}

/* Gets the next 32-bit integer using the cmwc algorithm. */
static uint32_t cmwc_next()
{
  uint64_t t, a = 123471786ULL;
  static uint32_t c = 362436, i = 1023;
  uint32_t x, r = 0xfffffffe;

  i = (i + 1) & 1023;
  t = a * Q[i] + c;
  c = (t >> 32);
  x = t + c;
  if (x < c) {
    x++;
    c++;
  }

  return (Q[i] = r - x);
}
