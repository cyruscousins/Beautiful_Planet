#ifndef PARAMETRIC_H
#define PARAMETRIC_H

#include "image.h"

typedef struct vec2 {
  float x, y;
} vec2;

//Generic functions for working with parametric curves.

//TODO generic function to advance along a parametric curve.

vec2 ddt(vec2 (*f)(float t, void* cl), float t, void* cl);

//Functions for drawing parametric curves.

void draw_parametric_curve_uniform_time(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float step, void* cl1, void* cl2);

void draw_parametric_curve_uniform_space(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float s0, float spacing, float tolerance, void* cl1, void* cl2);

//void trace_parametric_curve(image* img void (*draw)(float x, float y, void* cl), vec2 (*f)(float t), float t0, float t1, float step, void* cl1, void* cl2);

typedef struct ccl_1 {
  float x0, y0, theta0;
  float base, amplitude, frequency;
} ccl_1;

void randomize_ccl_1(ccl_1* ccl, float x0, float x1, float scale);

vec2 parametric_curve_1(float t, void* cl);

//Harmonograph based curves
//TODO would be more interesting to generalize.
typedef struct ccl_2 {
  float x0, y0, xS, yS;
  float d[4];
  float f[4];
  float p[4];
} ccl_2;

void randomize_ccl_2(ccl_2* ccl, float x0, float x1, float scale);

vec2 parametric_curve_2(float t, void* cl);

//vec2 noise_curve_transformer()

#endif
