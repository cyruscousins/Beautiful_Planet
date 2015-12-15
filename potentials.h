#ifndef POTENTIALS_H
#define POTENTIALS_H

//Force functions are represented as potential functions.

typedef struct centered_cl {
  float cx, cy;
  float strength;
  float denominatorSummand;
} centered_cl;

float distanceSquaredPotential(float x, float y, void* cl);

float noiseSumPotential(float x, float y, void* cl);

typedef struct poly_cl {
  float(**f)(float, float, void*);
  void** cl;
  unsigned count;
} poly_cl;

float sumPotential(float x, float y, void* cl);

typedef struct poly_weighted_cl {
  float(**f)(float, float, void*);
  void** cl;
  float* weight;
  unsigned count;
} poly_weighted_cl;

float sumWeightedPotential(float x, float y, void* cl);

#endif
