
#include <assert.h>
#include <math.h>

#include "wind.h"
#include "noise.h"
#include "parametric.h"

typedef char bool;

wind* wind_new(unsigned particleCount) {
  wind* w = malloc(sizeof(wind) + particleCount * DATA_SIZE * sizeof(float));
  w->particles = 0;
  w->maxParticles = particleCount;
  return w;
}

void wind_free(wind* w) {
  free(w);
}

void wind_update(wind* w, float t, float (*pressure)(float, float, void*), void* cl) {
  for(unsigned i = 0; i < w->particles; i++) {
    float p0 = pressure(wind_x(w)[i], wind_y(w)[i], cl);
    float px = pressure(wind_x(w)[i] + EPSILON, wind_y(w)[i], cl);
    float py = pressure(wind_x(w)[i], wind_y(w)[i] + EPSILON, cl);
    
    wind_dx(w)[i] += t * (px - p0) / (EPSILON * wind_mass(w)[i]);
    wind_dy(w)[i] += t * (py - p0) / (EPSILON * wind_mass(w)[i]);

    wind_x(w)[i] += t * wind_dx(w)[i];
    wind_y(w)[i] += t * wind_dy(w)[i];
  }
}

void wind_update_bound(wind* w, float t, float (*pressure)(float, float, void*), void* cl, float x0, float y0, float x1, float y1) {
  for(unsigned i = 0; i < w->particles; i++) {
    if(wind_x(w)[i] < x0 || wind_x(w)[i] > x1 || wind_y(w)[i] < y0 || wind_y(w)[i] > y1) {
      wind_remove(w, i);
      i--;
      continue;
    }
    
    float p0 = pressure(wind_x(w)[i], wind_y(w)[i], cl);
    float px = pressure(wind_x(w)[i] + EPSILON, wind_y(w)[i], cl);
    float py = pressure(wind_x(w)[i], wind_y(w)[i] + EPSILON, cl);
    
    wind_dx(w)[i] += t * (px - p0) / (EPSILON * wind_mass(w)[i]);
    wind_dy(w)[i] += t * (py - p0) / (EPSILON * wind_mass(w)[i]);

    wind_x(w)[i] += t * wind_dx(w)[i];
    wind_y(w)[i] += t * wind_dy(w)[i];
  }
}

void wind_scale_velocity(wind* w, float s) {
  for(unsigned i = 0; i < w->particles; i++) {
    wind_dx(w)[i] *= s;
    wind_dy(w)[i] *= s;
  }
}

//TODO may be good to replace a value if append fails.
void wind_append(wind* w, float x, float y, float dx, float dy, float mass) {
  if(w->particles < w->maxParticles) {
    wind_x(w)[w->particles] = x;
    wind_y(w)[w->particles] = y;
    wind_dx(w)[w->particles] = dx;
    wind_dy(w)[w->particles] = dy;
    wind_mass(w)[w->particles] = mass;
    w->particles++;
  }
}

void wind_remove(wind* w, unsigned index) {
  assert(index < w->particles);
  w->particles--;
  wind_x   (w)[index] = wind_x(w)[w->particles];
  wind_dx  (w)[index] = wind_dx(w)[w->particles];
  wind_y   (w)[index] = wind_y(w)[w->particles];
  wind_dy  (w)[index] = wind_dy(w)[w->particles];
  wind_mass(w)[index] = wind_mass(w)[w->particles];
}

//Removes particles, jumping [0, d) between removals.
void wind_remove_rand(wind* w, unsigned d) {
  unsigned i = 0;
  while(1) {
    i += uniformInt(0, d);
    if(i >= w->particles) break;
    wind_remove(w, i);
  }
}

void wind_empty(wind* w) {
  w->particles = 0;
}

#define PI 3.14159627
#define TAU (2*PI)
void wind_randomize(wind* w, unsigned count, float x0, float y0, float x1, float y1, float vMax, float mMax) {
  wind_empty(w);
  for(unsigned i = 0; i < count; i++) {
    float r = uniformFloat(0, vMax);
    float theta = uniformFloat(0, TAU);
    
    float dx = r * cosf(theta);
    float dy = r * sinf(theta);
    
    wind_append(w, uniformFloat(x0, x1), uniformFloat(y0, y1), dx, dy, uniformFloat(EPSILON, mMax));
  }
}

static bool bounded(float f, float bottom, float top) {
  return f >= bottom && f <= top;
}

static bool bounded01(float f) {
  return bounded(f, 0, 1);
}

//Drawing

void wind_draw(wind* w, image* img, float r, float g, float b, float a, float x0, float y0, float scale) {
  assert(bounded01(r) && bounded01(g) && bounded01(b) && bounded01(a));
  for(unsigned i = 0; i < w->particles; i++) {
    //Translate, scale, and offset to to make truncation round properly.
    float x = (wind_x(w)[i] - x0) / scale + 0.5;
    float y = (wind_y(w)[i] - y0) / scale + 0.5;
    if(x < 0 || x >= img->width || y < 0 || y >= img->height) continue;

    float* rv = image_pixel(img, (unsigned)x, (unsigned)y, R);
    float* gv = image_pixel(img, (unsigned)x, (unsigned)y, G);
    float* bv = image_pixel(img, (unsigned)x, (unsigned)y, B);

    *rv = *rv * (1 - a) + a * r;
    *gv = *gv * (1 - a) + a * g;
    *bv = *bv * (1 - a) + a * b;
  }
}
void wind_draw_roffset(wind* w, image* img, float r, float g, float b, float a, float x0, float y0, float scale, unsigned copies, float spread) {
  //assert(bounded01(r) && bounded01(g) && bounded01(b) && bounded01(a));
  for(unsigned i = 0; i < w->particles; i++) {
    //Translate, scale, and offset to to make truncation round properly.
    float xf = (wind_x(w)[i] - x0) / scale + 0.5;
    float yf = (wind_y(w)[i] - y0) / scale + 0.5;
    for(unsigned j = 0; j < copies; j++) {
      vec2 s = symmetricBall(spread);
      unsigned x = (unsigned)(xf + s.x);
      unsigned y = (unsigned)(yf + s.y);
      
      if(x < 0 || x >= img->width || y < 0 || y >= img->height) continue;

      float* rv = image_pixel(img, (unsigned)x, (unsigned)y, R);
      float* gv = image_pixel(img, (unsigned)x, (unsigned)y, G);
      float* bv = image_pixel(img, (unsigned)x, (unsigned)y, B);

      *rv = *rv * (1 - a) + a * r;
      *gv = *gv * (1 - a) + a * g;
      *bv = *bv * (1 - a) + a * b;
    }
  }
}


void wind_print(wind* w, FILE* f) {
  fprintf(f, "{WIND %d / %d: (x, y, dx, dy, m)\n", w->particles, w->maxParticles);
  for(unsigned i = 0; i < w->particles; i++) {
    fprintf(f, "%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n", wind_x(w)[i], wind_y(w)[i], wind_dx(w)[i], wind_dy(w)[i], wind_mass(w)[i]);
  }
  fprintf(f, "}\n");
}
