
#ifndef WIND_H
#define WIND_H

//#include <unistd.h>
#include <stdlib.h>

#include "image.h"
#include "draw.h"

typedef struct wind {
  unsigned particles;
  unsigned maxParticles;
  float data[];
} wind;

#define X_IDX 0
#define Y_IDX 1
#define DX_IDX 2
#define DY_IDX 3
#define MASS_IDX 4
#define DATA_SIZE 5

wind* wind_new(unsigned particleCount);
void wind_free(wind* w);

static float* wind_x(wind* w) {
  return w->data + (w->maxParticles * X_IDX);
}

static float* wind_y(wind* w) {
  return w->data + (w->maxParticles * Y_IDX);
}

static float* wind_dx(wind* w) {
  return w->data + (w->maxParticles * DX_IDX);
}

static float* wind_dy(wind* w) {
  return w->data + (w->maxParticles * DY_IDX);
}

static float* wind_mass(wind* w) {
  return w->data + (w->maxParticles * MASS_IDX);
}


//Modifiers
void wind_update(wind* w, float t, float (*pressure)(float, float, void*), void* cl);
void wind_update_bound(wind* w, float t, float (*pressure)(float, float, void*), void* cl, float x0, float y0, float x1, float y1);
void wind_scale_velocity(wind* w, float s);
void wind_append(wind* w, float x, float y, float dx, float dy, float mass);
void wind_remove(wind* w, unsigned index);
void wind_remove_rand(wind* w, unsigned d2);
void wind_empty(wind* w);
void wind_randomize(wind* w, unsigned count, float x0, float y0, float x1, float y1, float vMax, float mMax);

//Drawing
void wind_draw(wind* w, image* i, float r, float g, float b, float a, float x0, float y0, float scale);
void wind_draw_roffset(wind* w, image* i, float r, float g, float b, float a, float x0, float y0, float scale, unsigned copies, float spread);
void wind_draw_generic(wind* w, image* img, float x0, float y0, float scale, float bound, drawfun df, void* dcl);

void wind_print(wind* w, FILE* f);

#endif
