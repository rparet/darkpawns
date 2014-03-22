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

#include "random.h"

static void cmwc_seed(unsigned long seed);
static unsigned long cmwc_next();

/* Generic entry point to the RNG's seeding function. */
void prng_seed(unsigned long seed)
{
  cmwc_seed(seed);
}

/* Get the next 32-bit integer in the random sequence. */
unsigned long prng_next()
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
static unsigned long Q[1024];

/* Seeds the cmwc RNG using xorshift. */
static void cmwc_seed(unsigned long j)
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
static unsigned long cmwc_next()
{
  unsigned long long t, a = 123471786ULL;
  static unsigned long c = 362436, i = 1023;
  unsigned long x, r = 0xfffffffe;

  i = (i + 1) & 1023;
  t = a * Q[i] + c;
  c = (t >> 32);
  x = t + c;
  if (x < c) {
    x++;
    c++;
  }

  return (unsigned int) (Q[i] = r - x);
}
