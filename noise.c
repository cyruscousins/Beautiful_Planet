#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "global.h"
#include "noise.h"

#include "convolution.h"
#include "float.h"

//Here's an LCG.  Using rand() has a lot of overhead.
uint32_t lcg = 0x12191989;
void seed_lcg(uint32_t seed) {
  lcg = seed;
}
unsigned rand_lcg_global() {
  lcg = 69069 * lcg;
  return ++lcg;
}

unsigned lcg_max = 0xffffffff;

//Basic random functionality based on the LCG.
float rfloat() {
  return rand_lcg_global() / (float)lcg_max;
}
int uniformInt(int x0, int x1) {
  assert(x0 <= x1);
  return x0 + rand_lcg_global() % (x1 - x0);
}
unsigned uniformNatural(int top) {
  return rand_lcg_global() % top;
}
float uniformFloat(float f0, float f1) {
  assert(f0 <= f1);
  return rfloat() * (f1 - f0) + f0;
}
float uniformFloatS(float f) {
  return uniformFloat(-f, f);
}

vec2 uniformUnitCirc() {
  float theta = uniformFloat(0, TAU);
  vec2 result = { cosf(theta), sinf(theta) };
  return result;
}
vec2 symmetricUnitBall() {
  return vScale(rfloat(), uniformUnitCirc());
}
vec2 symmetricBall(float radius) {
  return vScale(rfloat() * radius, uniformUnitCirc());
}


//Noise functions:

float* noise_2d_value(noise* n, unsigned x, unsigned y) {
  x = x % n->count;
  y = y % n->count;
  return &n->values[x + y * n->count];
}

//Randomly selects half the pixels (with replacement) and blurs them evenly with their neighbors.  Note: this function is lagely subsumed by convolution.
void blur_noise(noise* n, unsigned dimension, float fraction) {
  assert(dimension == 2); //TODO hardcoded for dimension 2.  Make this generic.
  assert(!(n->count & 1)); //No odd noise functions.
  unsigned totalCount = upow(n->count, dimension);
  unsigned numToBlur = uroundf(fraction * totalCount);
  for(unsigned i = 0; i < numToBlur; i++) {
    unsigned x = uniformNatural(n->count);
    unsigned y = uniformNatural(n->count);
    *noise_2d_value(n, x, y) = (
      *noise_2d_value(n, x, y) +
      *noise_2d_value(n, x + 1, y) +
      *noise_2d_value(n, x, y + 1) +
      *noise_2d_value(n, x - 1, y) +
      *noise_2d_value(n, x, y - 1)
    ) / 5;
  }
}

void noise_rescale_out(noise* n, float nmin, float nmax) {
  assert(nmin <= nmax);
  unsigned count = n->count;
  float tmin = n->values[0];
  float tmax = n->values[0];
  for(unsigned i = 1; i < count; i++) {
    if(n->values[i] > tmax) {
      tmax = n->values[i];
    } else if(n->values[i] < tmin) {
      tmin = n->values[i];
    }
  }
  float scale;
  if(tmax == tmin) {
    scale = 0;
  } else {
    scale = (nmax - nmin) / (tmax - tmin);
  }
  for(unsigned i = 0; i < count; i++) {
    n->values[i] = (n->values[i] - tmin) * scale + nmin;
    assert(n->values[i] <= nmax + EPSILON);
    assert(n->values[i] >= nmin - EPSILON);
  }
}

//Saturate the noise toward the extremes.  Applies the following sigmoid curve:
/*
f(x) = { x < 0.5 : 2 * x * x | x > 0.5 : 1 - (2 * (1 - x) * (1 - x))
*/
float saturate(float f) {
  assert(f >= 0 - EPSILON && f <= 1 + EPSILON);
  
  if(f <= 0.5) {
    return 2 * f * f;
  }
  return 1 - 2 * (1 - f) * (1 - f);
}
void float_array_saturate(float* f, unsigned count, float saturation) {
  for(unsigned i = 0; i < count; i++) {
    f[i] = f[i] * (1 - saturation) + saturate(f[i]) * saturation;
  }
}
void noise_saturate(noise* n, float saturation) {
  float_array_saturate(n->values, n->count, saturation);
}

void initialize_noise_1d_inplace(noise* n, unsigned count) {
  n->count = count;
  for(unsigned i = 0; i < count; i++) {
    n->values[i] = rfloat();
    //printf("%f %d %d %f\n", n->values[i], rand(), RAND_MAX, rand() / (float)RAND_MAX);
  }
}

noise* initialize_noise_1d(unsigned count) {
  noise* n = malloc(sizeof(noise) + count * sizeof(float));
  initialize_noise_1d_inplace(n, count);
  return n;
}

noise* initialize_noise_2d(unsigned count) {
  noise* n = initialize_noise_1d(count * count);
  n->count = count;
  blur_noise(n, 2, 0.5);
  return n;
}

noise* initialize_noise_3d(unsigned count) {
  noise* n = initialize_noise_1d(count * count * count);
  n->count = count;
  return n;
}

void noise_free(noise* n) {
  free(n);
}

float noise1d(float x, noise* n) {
  long lx = (long)x;
  if(x < 0) lx--;
  float fx = (x - lx);
  
  float f0 = n->values[lx % n->count];
  float f1 = n->values[(lx + 1) % n->count];

  return f0 * (1 - fx) + f1 * fx;
}

//#include <stdio.h>
//TODO more efficient version could duplicate borders, elimiating the need for %.
float noise2d(float x, float y, noise* n) {
  //TODO long can overflow here.
  if(x < 0) {
    x += n->count * (1 + ((long)(-x)) / n->count);
  }
  //assert(x >= 0);
  if(y < 0) {
    y += n->count * (1 + ((long)(-y)) / n->count);
  }
  //assert(y >= 0);
  long lx = (long)x;
  float fx = (x - lx);

  long ly = (long)y;
  float fy = (y - ly);
  
  float f00 = n->values[(lx + 0) % n->count + (ly % n->count) * n->count];
  float f01 = n->values[(lx + 0)  % n->count + ((ly + 1) % n->count) * n->count];
  float f10 = n->values[(lx + 1) % n->count + ((ly + 0) % n->count) * n->count];
  float f11 = n->values[(lx + 1) % n->count + ((ly + 1) % n->count) * n->count];
  
  return (f00 * (1 - fy) + f01 * fy) * (1 - fx) + (f10 * (1 - fy) + f11 * fy) * fx;
}


noise_sum* initialize_noise_sum_2d(unsigned noiseSize, unsigned count) {
  noise_sum* ns = malloc(sizeof(noise_sum) + noiseSize * noiseSize * sizeof(float));
  
  initialize_noise_1d_inplace(&(ns->n), noiseSize * noiseSize);
  //Blur with a convolution kernel.
  //convolve_kernel_blur_33_inplace(ns->n.values, noiseSize);
  convolve_kernel_blur_inplace(ns->n.values, noiseSize, 1 + noiseSize / 3); //Blur quite heavily.
  //Rescale to maximal range
  noise_rescale_out(&ns->n, 0, 1);
  //Saturate within the [0, 1] range (to favor extrema).
  noise_saturate(&ns->n, 0.5);
  
  ns->n.count = noiseSize;
  ns->count = count;
  
  ns->scale = noiseSize;
  return ns;
}

void noise_sum_free(noise_sum* ns) {
  free(ns);
}

void noise_sum_scale_in(noise_sum* n, float scale) {
  n->scale *= scale;
}

float noise_sum_2d(float x, float y, noise_sum* n) {
  float scale = n->scale;
  float v = 0;
  unsigned count = n->count;
  noise* noise = &n->n;

  for(unsigned i = 0; i < count; i++) {
    float tx = scale * x;
    float ty = scale * y;
    
    //float ttx = (tx * (count - i) + ty * i) / count;
    //float tty = (ty * (count - i) + tx * i) / count;
    //v += noise2d(ttx, tty, noise);
    //scale *= -1.5;
    
    v += noise2d(tx, ty, noise);
    scale *= -2;
  }
  return v / count;
  
  //Matrix version.  It's too slow.
  /*
  float mStore[8];
  memcpy(mStore, n->m, 8 * sizeof(float));

  float* m = mStore;
  float* nm = mStore + 4;
  
  for(unsigned i = 0; i < count; i++) {
    float tx = n->xp * i + m[0] * x + m[1] * y;
    float ty = n->yp * i + m[2] * x + m[3] * y;
    v += noise2d(ty, tx, noise);
    //Multiply m by the original matrix (we want m^i in the next iteration).
    nm[0] = m[0] * n->m[0] + m[1] * n->m[2];
    nm[1] = m[0] * n->m[1] + m[1] * n->m[3];
    //TODO nm2, nm3.
    if(m == mStore) {
      m = mStore + 4;
      nm = mStore;
    } else {
      m = mStore;
      nm = mStore + 4;
    }
  }
  return v / count;
   */
}

//This function takes a noise function onto a "nearby" (in terms of function distance) noise function.
//Given the noise function, its dimension, its bounds, a uniform distribution of perturbation [-amt, amt], the fraction of points to be perturbed, and the amount of saturation to apply on renormalization.
//The exact bound is extremely difficult to calculate, but for saturation = 1, assuming fraction * number of samples is an integer, and assuming no sample is taken out of bounds, the maximum change to the integral over the unit hypercube is fraction * amt.
void perturb_noise_sum(noise_sum* n, unsigned dimension, float n0, float n1, float amt, float fraction, float saturation) {
  assert(n0 <= n1);
  unsigned ncount = upow(n->n.count, dimension);
  unsigned perturbCt = uceilf(ncount * fraction + 1.0 - EPSILON);
  for(unsigned i = 0; i < perturbCt; i++) {
    unsigned idx = uniformNatural(ncount);
    //This check should never be violated.  However, we may want to be more tolerant in the future: if this is violated for good reasons, we may want to remove the check.
    assert(n->n.values[idx] >= n0 && n->n.values[idx] <= n1);
    n->n.values[idx] = n->n.values[idx] + uniformFloatS(amt);
  }
  
  //Apply some blur
  blur_noise(&n->n, dimension, 0.25);
  
  //Convert to 1d noise.
  unsigned ocount = n->n.count;
  n->n.count = ncount;
  
  //Normalize
  noise_rescale_out(&n->n, n0, n1);
  
  //Saturate
  noise_saturate(&n->n, saturation);
  
  //Convert back to d dimensional noise.
  n->n.count = ocount;
}

