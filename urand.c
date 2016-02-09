#include <R.h>
#include <Rmath.h>
#include <R_ext/Random.h>
#include "omprnd.h"

static Int32 seed;
static double * res;

double * user_unif_rand() {
#ifdef _OPENMP
  unsigned int tn;
  tn = omp_get_thread_num();
  res[tn] = genrand();
  return &res[tn];
#else
  *res = genrand();
  return res;
#endif
}

// user_norm_rand

void  user_unif_init(Int32 seed_in) {
  seed = seed_in;
  sgenrand((unsigned long) seed_in);
#ifdef _OPENMP
  int nth;
#pragma omp parallel default(shared)
{
  #pragma omp master
  nth = 624 * omp_get_num_threads();
}
  if ((res = (double *) malloc(nth * sizeof(double))) == NULL)
      error("Results vector cannot be allocated");
#else
  if ((res = (double *) malloc(sizeof(double))) == NULL)
      error("Results vector cannot be allocated");
#endif
}

static int nseed = 0;
#ifdef _OPENMP
static int n_omp_seed = 0;
#endif
int * user_unif_nseed() {
    nseed = 624;
#ifdef _OPENMP
#pragma omp parallel default(shared)
{
  #pragma omp master
  n_omp_seed = 624 * omp_get_num_threads();
}
#endif
  return &nseed;
}

int * user_unif_seedloc() { return (int *) mt; }
