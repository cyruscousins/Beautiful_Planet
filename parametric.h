#ifndef PARAMETRIC_H
#define PARAMETRIC_H

#include "image.h"

typedef struct vec2 {
  float x, y;
} vec2;

vec2 vPlus(vec2 a, vec2 b);
vec2 vMinus(vec2 a, vec2 b);
vec2 vScale(float s, vec2 a);
float vDot(vec2 a, vec2 b);

float vDSqr(vec2 a, vec2 b);
float vDistance(vec2 a, vec2 b);

vec2 vRotate(vec2 a, float theta);

vec2 uniformUnitCirc();
vec2 symmetricUnitBall();
vec2 symmetricBall(float radius);

//Generic functions for working with parametric curves.

//TODO generic function to advance along a parametric curve.

vec2 ddt(vec2 (*f)(float t, void* cl), float t, void* cl);

//Functions for drawing parametric curves.

void draw_parametric_curve_uniform_time(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float step, void* cl1, void* cl2);

void draw_parametric_curve_uniform_space(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float s0, float spacing, float tolerance, void* cl1, void* cl2);

//void trace_parametric_curve(image* img void (*draw)(float x, float y, void* cl), vec2 (*f)(float t), float t0, float t1, float step, void* cl1, void* cl2);

typedef struct ccl_1 {
  float x0, y0, theta0;
  float base, amplitude;
  unsigned frequency;
} ccl_1;

void randomize_ccl_1(ccl_1* ccl, float x0, float x1, float scale);

vec2 parametric_curve_1(float t, void* cl);

#define PC2_SUMMANDS 4
//Harmonograph based curves
//TODO would be more interesting to generalize.
typedef struct ccl_2 {
  float x0, y0, scale, theta;
  float d[PC2_SUMMANDS];
  float f[PC2_SUMMANDS];
  float p[PC2_SUMMANDS];
} ccl_2;

void randomize_ccl_2(ccl_2* ccl, float x0, float x1, float scale);

vec2 parametric_curve_2(float t, void* cl);

//vec2 noise_curve_transformer()

//Used for hypocycloids and epicycloids.
typedef struct cycloid {
  float x0, y0, theta0;
  float rb, r, a;
} cycloid;

void randomize_hypocycloid(cycloid* ccl, float x0, float y0, float scale);
void randomize_epicycloid(cycloid* ccl, float x0, float y0, float scale);

vec2 hypocycloid(float t, void* ccl);
vec2 epicycloid(float t, void* ccl);

//Weighted sums of parametric curves.
typedef struct weighted_sum_pcl {
  void** cl;
  vec2 (**f)(float, void*);
  float* weights;
  float* timeFactors;
  float* timeSummands;
  unsigned count;
} weighted_sum_pcl;

vec2 parametric_curve_point(float t, void* cl);

vec2 parametric_curve_weighted_sum(float t, void* cl);

weighted_sum_pcl* randomize_weighted_sum(float x0, float y0, float scale, unsigned maxComponents);
void free_weighted_sum(weighted_sum_pcl* w);

#endif

