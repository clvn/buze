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
// ----------- header -----------------------------------------------
// pinkgen.h

#ifndef  _pinkgen_h_
#define  _pinkgen_h_  1

#include  <stddef.h>
#include  <stdlib.h>

// You must provide the uniform random generator class.

class PinkNoise {
  private:
    // Coefficients (fixed)
    static long int const pA[5];
    static short int const pPSUM[5];

    // Internal pink generator state
    long int   contrib[5];   // stage contributions
    long int   accum;        // combined generators
    void       internal_clear( );          

    // Include a UNoise component
    //URand     ugen;

  public:
    PinkNoise( );
	~PinkNoise( ) {}
    void  pinkclear( );
    short int  pinkrand( );
} ;
#endif