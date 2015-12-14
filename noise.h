#ifndef NOISE_H
#define NOISE_H

#include <stdlib.h>

float rfloat();
int uniformInt(int x0, int x1);
float uniformFloat(float f0, float f1);

typedef struct noise {
  unsigned count;
  float values[];
} noise;

noise* initialize_noise_1d(unsigned count);
noise* initialize_noise_2d(unsigned count);
noise* initialize_noise_3d(unsigned count);

float noise1d(float x, noise* n);
float noise2d(float x, float y, noise* n);


typedef struct noise_sum {
  unsigned count;
  float scale;
  noise n;
} noise_sum;

noise_sum* initialize_noise_sum_2d(unsigned size, unsigned count);

//Scales given the noise sum function.
void noise_sum_scale_in(noise_sum* n, float scale);

float noise_sum_2d(float x, float y, noise_sum* n);

#endif
