// Pink noise class using the autocorrelated generator method.
// Method proposed and described by Larry Trammell "the RidgeRat" --
// see http://home.earthlink.net/~ltrammell/tech/newpink.htm
// There are no restrictions.
//
// ------------------------------------------------------------------
//
// This is a canonical, 16-bit fixed-point implementation of the
// generator in 32-bit arithmetic. There are only a few system
// dependencies.
//
//   -- access to an allocator 'malloc' for operator new
//   -- access to definition of 'size_t'
//   -- assumes 32-bit two's complement arithmetic
//   -- assumes long int is 32 bits, short int is 16 bits
//   -- assumes that signed right shift propagates the sign bit
//
// It needs a separate URand class to provide uniform 16-bit random
// numbers on interval [1,65535]. The assumed class must provide
// methods to query and set the current seed value, establish a
// scrambled initial seed value, and evaluate uniform random values.
//
//
// ----------- implementation ---------------------------------------
// pinkgen.cpp

#include <time.h>

#include  "pinknoise.h"

// Static class data
long int const PinkNoise::pA[5] =
    { 14055, 12759, 10733, 12273, 15716 };
short int const PinkNoise::pPSUM[5] =
    { 22347, 27917, 29523, 29942, 30007 };

// Clear generator to a zero state.
void   PinkNoise::pinkclear( )
{
    int  i;
    for  (i=0; i<5; ++i)  { contrib[i]=0L; }
    accum = 0L;
}

// PRIVATE, clear generator and also scramble the internal
// uniform generator seed.
void   PinkNoise::internal_clear( )
{
    pinkclear();
	srand(time(0));
    //ugen.seed(0);    // Randomizes the seed!
}

// Constructor. Guarantee that initial state is cleared
// and uniform generator scrambled.
PinkNoise::PinkNoise( )
{
    internal_clear();
}

// Coding artifact for convenience
#define   UPDATE_CONTRIB(n)  \
     {                                   \
       accum -= contrib[n];              \
       contrib[n] = (long)randv * pA[n]; \
       accum += contrib[n];              \
       break;                            \
     }                                  

// Evaluate next randomized 'pink' number with uniform CPU loading.
short int   PinkNoise::pinkrand( )
{
    short int  randu = rand();            // U[0,32767]
    short int  randv = 2*rand() - 32768;  // U[-32768,32767]

    // Structured block, at most one update is performed
    while (1)
    {
      if (randu < pPSUM[0]) UPDATE_CONTRIB(0);
      if (randu < pPSUM[1]) UPDATE_CONTRIB(1);
      if (randu < pPSUM[2]) UPDATE_CONTRIB(2);
      if (randu < pPSUM[3]) UPDATE_CONTRIB(3);
      if (randu < pPSUM[4]) UPDATE_CONTRIB(4);
      break;
    }
    return (short int) (accum >> 16);
}
