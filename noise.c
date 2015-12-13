#include <stdlib.h>
#include <assert.h>

#include "noise.h"
//#include <stdio.h>

float rfloat() {
  return rand() / (float)RAND_MAX;
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

//TODO replace with fast in place convolution.
void blur_noise_2d(noise* n){
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





noise* initialize_noise_1d(unsigned count) {
  noise* n = malloc(sizeof(noise) + count * sizeof(float));
  n->values = (float*) (n + 1);
  n->count = count;
  for(unsigned i = 0; i < count; i++) {
    n->values[i] = rfloat();
    //printf("%f %d %d %f\n", n->values[i], rand(), RAND_MAX, rand() / (float)RAND_MAX);
  }
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

float noise1d(float x, noise* n) {
  long lx = (long)x;
  if(x < 0) lx--;
  float fx = (x - lx);
  
  float f0 = n->values[lx % n->count];
  float f1 = n->values[(lx + 1) % n->count];

  return f0 * (1 - fx) + f1 * fx;
}

#include <stdio.h>
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


noise_sum* initialize_noise_sum_2d(unsigned size, unsigned count) {
  noise** noises = malloc(sizeof(noise) * count);
  float* scales = malloc(sizeof(float) * count);

  noises[0] = initialize_noise_2d(size);
  scales[0] = size;

  for(unsigned i = 1; i < count; i++) {
    noises[i] = noises[i - 1];
    scales[i] = scales[i - 1] * (1 + rfloat());
  }

  /*
  for(unsigned i = 0; i < count; i++) {
    noises[i] = initialize_noise_2d(size);
    scales[i] = 1.0 / (1 << i);
  }
  */

  noise_sum* n = malloc(sizeof(noise_sum));
  n->noises = noises;
  n->scales = scales;
  n->count = count;
  return n;
}

void noise_sum_scale_in(noise_sum* n, float scale) {
  for(unsigned i = 0; i < n->count; i++) {
    n->scales[i] *= scale;
  }
}

float noise_sum_2d(float x, float y, noise_sum* n) {
  float v = 0;
  for(unsigned i = 0; i < n->count; i++) {
    v += noise2d(n->scales[i] * x, n->scales[i] * y, n->noises[i]);
  }
  return v / n->count;
}

float noise_weighted_sum_2d(float x, float y, noise_sum* n) {
  float v = 0;
  float w = 0;
  unsigned count = n->count;
  for(unsigned i = 0; i < count; i++) {
    v += (1 / n->scales[i]) * noise2d(n->scales[i] * x, n->scales[i] * y, n->noises[i]);
    w += 1 / n->scales[i];
  }
  return v / w;
}
