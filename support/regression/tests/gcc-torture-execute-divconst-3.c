/*
divconst-3.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

long long
f (long long x)
{
  return x / 10000000000LL;
}

void
testTortureExecute (void)
{
#ifndef __SDCC_mcs51
#ifndef __SDCC_ds390
  if (f (10000000000LL) != 1 || f (100000000000LL) != 10)
    ASSERT (0);
  return;
#endif
#endif
}
