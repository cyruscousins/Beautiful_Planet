//Force functions are represented as potential functions.

#include "potentials.h"
#include "noise.h"

float distanceSquaredPotential(float x, float y, void* cl) {
  centered_cl* ocl = cl;
  float dx = x - ocl->cx;
  float dy = y - ocl->cy;
  
  float distanceSquared = dx * dx + dy * dy;
  
  return ocl->strength / (distanceSquared + ocl->denominatorSummand);
}

float noiseSumPotential(float x, float y, void* cl) {
  return noise_sum_2d(x, y, cl);
}

float sumPotential(float x, float y, void* cl) {
  poly_cl* pcl = cl;
  float sum = 0;
  for(unsigned i = 0; i < pcl->count; i++) {
    sum += (*(pcl->f))(x, y, pcl->cl[i]);
  }
  return sum;
}

float gradientPotential(float x, float y, void* cl) {
  vec2* v = cl;
  return v->x * x + v->y * y;
}

float sumWeightedPotential(float x, float y, void* cl) {
  poly_weighted_cl* pcl = cl;
  float sum = 0;
  for(unsigned i = 0; i < pcl->count; i++) {
    sum += pcl->weight[i] * (pcl->f[i])(x, y, pcl->cl[i]);
  }
  return sum;
}

