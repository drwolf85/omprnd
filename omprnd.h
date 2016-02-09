/* A C-program for MT19937: Real number version                */
/* genrand() generates one pseudorandom real number (double)   */
/* which is uniformly distributed on [0,1]-interval, for each  */
/* call. sgenrand(seed) set initial values to the working area */
/* of 624 words. Before genrand(), sgenrand(seed) must be      */
/* called once. (seed is any 32-bit integer except for 0).     */
/* Integer generator is obtained by modifying two lines.       */
/* Coded by Takuji Nishimura, considering the suggestions by   */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997. Comments  */
/* should be addressed to: matumoto@math.keio.ac.jp            */

/* Modified by Luca Sartore on May 13, 2012 */

//#include <stdlib.h>
#include <R.h>
#ifdef _OPENMP
#include <omp.h>
#endif

void getNumCores(int *);
void getNumSlaves(int *);
void setNumSlaves(int *);
void SGR(int *);
void sgenrand(unsigned long);
double genrand();

static unsigned long *mt; /* the array for the state vector */
static int mti = 625; /* mti==625 means mt[624] is not initialized */
static int prevAlloc = 0;

#ifdef _OPENMP
#pragma omp threadprivate(mti)
#endif

void getNumCores(int *n) {
  /* Get the max number of CPU cores
          *n - num of cores */
  #ifdef _OPENMP
    *n = omp_get_num_procs();
  #else
    *n = 1;
  #endif
}

void getNumSlaves(int *n) {
  /* Get the number of threads to use
          *n - num of threads */
  #ifdef _OPENMP
    #pragma omp parallel default(shared)
    {
      #pragma omp master
        *n = omp_get_num_threads();
    }
  #else
    *n = 1;
  #endif
}

void setNumSlaves(int *n) {
  /* Set the number of threads to use
          *n - num of threads */
  #ifdef _OPENMP
    if (omp_get_num_procs() < *n) {
      *n = omp_get_num_procs();
      omp_set_num_threads(*n);
    }
    else if(*n > 0) {
      omp_set_num_threads(*n);
    }
    else {
      omp_set_num_threads(1);
      *n = 1;
    }
  #else
    *n = 0;
  #endif
}

void SGR(int *myseed) {
  /* Initializing the array with a NONZERO seed from R 
       myseed - integer number as seed */
  sgenrand((unsigned long) *myseed);
}

void sgenrand(unsigned long seed) {
  /* Initializing the array with a NONZERO seed */
  /* setting initial seeds to mt[624] using        */
  /* the generator Line 25 of Table 1 in           */
  /* [KNUTH 1981, The Art of Computer Programming  */
  /* Vol. 2 (2nd Ed.), pp102]                      */
  /* Parallel version: Luca Sartore Dec. 2011 - drwolf85@gmail.com */
  int i, nth;

#ifdef _OPENMP
  #pragma omp parallel default(shared)
  {
    nth = 624 * omp_get_num_threads();
  }
#else
  nth = 624;
#endif
  if (!prevAlloc) {
    if ((mt = (unsigned long *) calloc(nth, sizeof(long))) == NULL) {
      error("Seed vector cannot be allocated");
    }
    prevAlloc = !prevAlloc;
  }
  if (!seed) seed = 4357;

  mt[0] = seed & 0xffffffff;
  for (i = 1; i < nth; i++)
    mt[i] = (69069 * mt[i - 1]) & 0xffffffff;

#ifdef _OPENMP
  #pragma omp parallel
  {
#endif
      mti = 0;
#ifdef _OPENMP
  }
#endif
}

double genrand() {
  /* Generating pseudo-random numbers for a Uniform between 0 and 1 */
#ifdef _OPENMP
  int nth = 624 * omp_get_thread_num();
#else
  int nth = 624;
#endif
  unsigned long y;
  static unsigned long mag01[2] = {0x0, 0x9908b0df};
  /* mag01[x] = x * 0x9908b0df for x=0,1 */

  if (mti >= 624) { /* generate 624 words at one time */
    int kk;

/* FIXME if sgenrand() has not been called, 
   the use of a default initial seed produce an error */
    if (mti == 625) /* if sgenrand() has not been called, */
      sgenrand(4357); /* a default initial seed is used */
    
    for (kk = 0; kk < 227; kk++) {
      y = (mt[nth + kk] & 0x80000000)|(mt[nth + kk + 1] & 0x7fffffff);
      mt[nth + kk] = mt[nth + kk + 397] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (; kk < 623; kk++) {
      y = (mt[nth + kk]&0x80000000)|(mt[nth + kk + 1] & 0x7fffffff);
      mt[nth + kk] = mt[nth + kk - 227] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[nth + 623] & 0x80000000)|(mt[nth + 0] & 0x7fffffff);
    mt[nth + 623] = mt[nth + 396] ^ (y >> 1) ^ mag01[y & 0x1];
    mti = 0;
  }

  y = mt[nth + mti++];
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680;
  y ^= (y << 15) & 0xefc60000;
  y ^= (y >> 18);

  return ( (double) y / (unsigned long) 0xffffffff ); /* reals */
  /* return y; */ /* for integer generation */
}

