#include <stdlib.h>
#include <assert.h>

#include "noise.h"
#include "convolution.h"
//#include <stdio.h>

#define EPSILON 0.0001

//Here's an LCG.  Using rand() has a lot of overhead.
uint32_t lcg = 12191989;
void seed_lcg(uint32_t seed) {
  lcg = seed;
}
unsigned rand_lcg_global() {
  lcg = 69069 * lcg;
  return ++lcg;
}

unsigned lcg_max = 0xffffffff;

float rfloat() {
  return rand_lcg_global() / (float)lcg_max;
}

int uniformInt(int x0, int x1){
  assert(x0 <= x1);
  return x0 + rand() % (x1 - x0);
}

float uniformFloat(float f0, float f1) {
  assert(f0 <= f1);
  return rfloat() * (f1 - f0) + f0;
}



float* noise_2d_value(noise* n, unsigned x, unsigned y) {
  x = x % n->count;
  y = y % n->count;
  return &n->values[x + y * n->count];
}

//Randomly selects half the pixels (with replacement) and blurs them evenly with their neighbors.  Note: this function is lagely subsumed by convolution.
void blur_noise_2d(noise* n) {
  for(unsigned i = 0; i < n->count * n->count / 2; i++) {
    unsigned x = rand() % n->count;
    unsigned y = rand() % n->count;
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
void noise_saturate(noise* n, float saturation) {
  for(unsigned i = 0; i < n->count; i++) {
    n->values[i] = n->values[i] * (1 - saturation) + saturate(n->values[i]) * saturation;
  }
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
  blur_noise_2d(n);
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
  convolve_kernel_blur_inplace(ns->n.values, noiseSize, 1 + noiseSize / 4); //Blur quite heavily.
  //Rescale to maximal range
  noise_rescale_out(&ns->n, 0, 1);
  //Saturate within the [0, 1] range (to favor extrema).
  noise_saturate(&ns->n, 0.5);
  
  ns->n.count = noiseSize;
  //blur_noise_2d(&(ns->n));
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
    v += noise2d(scale * x, scale * y, noise);
    scale *= -2;
  }
  return v / count;
}

