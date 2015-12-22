#ifndef NOISE_H
#define NOISE_H

#include "global.h"

#include <stdlib.h>
#include <inttypes.h>

void seed_lcg(uint32_t seed);
unsigned rand_lcg_global();

float rfloat();
int uniformInt(int x0, int x1);
float uniformFloat(float f0, float f1);
float uniformFloatS(float f);

vec2 uniformUnitCirc();
vec2 symmetricUnitBall();
vec2 symmetricBall(float radius);

typedef struct noise {
  unsigned count;
  float values[];
} noise;

noise* initialize_noise_1d(unsigned count);
noise* initialize_noise_2d(unsigned count);
noise* initialize_noise_3d(unsigned count);

void noise_free(noise* n);

float noise1d(float x, noise* n);
float noise2d(float x, float y, noise* n);


typedef struct noise_sum {
  unsigned count;
  float scale;
  noise n;
} noise_sum;

noise_sum* initialize_noise_sum_2d(unsigned size, unsigned count);

void noise_sum_free(noise_sum* ns);

//Scales given the noise sum function.
void noise_sum_scale_in(noise_sum* n, float scale);

float noise_sum_2d(float x, float y, noise_sum* n);

void perturb_noise_sum(noise_sum* n, unsigned dimension, float n0, float n1, float amt, float fraction, float saturation);

#endif
