
#ifndef WIND_H
#define WIND_H

//#include <unistd.h>
#include <stdlib.h>

#include "image.h"

//TODO no need for data pointer.
typedef struct wind {
  float* data;
  unsigned particles;
  unsigned maxParticles;
} wind;

#define X_IDX 0
#define Y_IDX 1
#define DX_IDX 2
#define DY_IDX 3
#define MASS_IDX 4
#define DATA_SIZE 5

#define EPSILON 0.001

static wind* wind_new(unsigned particleCount) {
  wind* w = malloc(sizeof(wind) + particleCount * DATA_SIZE * sizeof(float));
  w->data = (float*) (w + 1);
  w->particles = 0;
  w->maxParticles = particleCount;
  return w;
}

static void wind_free(wind* w) {
  free(w);
}

static float* wind_x(wind w) {
  return w.data + (w.maxParticles * X_IDX);
}

static float* wind_y(wind w) {
  return w.data + (w.maxParticles * Y_IDX);
}

static float* wind_dx(wind w) {
  return w.data + (w.maxParticles * DX_IDX);
}

static float* wind_dy(wind w) {
  return w.data + (w.maxParticles * DY_IDX);
}

static float* wind_mass(wind w) {
  return w.data + (w.maxParticles * MASS_IDX);
}


//Modifiers
void wind_update(wind* w, float t, float (*pressure)(float, float, void*), void* cl);
void wind_update_bound(wind* w, float t, float (*pressure)(float, float, void*), void* cl, float x0, float y0, float x1, float y1);
void wind_append(wind* w, float x, float y, float dx, float dy, float mass);
void wind_remove(wind* w, unsigned index);
void wind_empty(wind* w);
void wind_randomize(wind* w, unsigned count, float x0, float y0, float x1, float y1, float vMax, float mMax);

//Drawing
void wind_draw(wind* w, image* i, float r, float g, float b, float a, float x0, float y0, float scale);
void wind_draw_roffset(wind* w, image* i, float r, float g, float b, float a, float x0, float y0, float scale, unsigned copies, unsigned maxOffset);

void wind_print(wind* w, FILE* f);

#endif