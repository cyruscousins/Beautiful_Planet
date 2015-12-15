#include "parametric.h"
#include "noise.h"

#include <math.h>

#include <stdio.h>

#define PI 3.14159627
#define TAU (2*PI)
#define EPSILON 0.0000001

//Generic functions

float vL2Sqr(vec2 a) {
  return a.x * a.x + a.y * a.y;
}
float vL1(vec2 a) {
  return fabs(a.x) + fabs(a.y);
}

vec2 vPlus(vec2 a, vec2 b) {
  vec2 result = { a.x + b.x, a.y + b.y };
  return result;
}
vec2 vMinus(vec2 a, vec2 b) {
  vec2 result = { a.x - b.x, a.y - b.y };
  return result;
}
vec2 vScale(float s, vec2 a) {
  vec2 result = { a.x * s, a.y * s };
  return result;
}
float vDot(vec2 a, vec2 b) {
  return a.x * b.x + a.y * b.y;
}

vec2 vRotate(vec2 a, float theta) {
  float cost = cosf(theta);
  float sint = sinf(theta);
  vec2 result = {a.x * cost - a.y * sint, a.x * sint + a.y * cost};
  return result;
}

float vDSqr(vec2 a, vec2 b) {
  return vL2Sqr(vMinus(a, b));
}
float vDistance(vec2 a, vec2 b) {
  return sqrtf(vL2Sqr(vMinus(a, b)));
}


vec2 uniformUnitCirc() {
  float theta = uniformFloat(0, TAU);
  vec2 result = { cosf(theta), sinf(theta) };
  return result;
}
vec2 symmetricUnitBall() {
  return vScale(rfloat(), uniformUnitCirc());
}
vec2 symmetricBall(float radius) {
  return vScale(rfloat() * radius, uniformUnitCirc());
}


vec2 ddt(vec2 (*f)(float t, void* cl), float t, void* cl) {
  vec2 v0 = f(t, cl);
  vec2 v1 = f(t + EPSILON, cl);
  return vScale(1 / EPSILON, vMinus(v1, v0));
}

//Drawing functions

void draw_parametric_curve_uniform_time(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float step, void* cl1, void* cl2) {
  for(float t = t0; t < t1; t += step) {
    vec2 v = f(t, cl2);
    
    draw(img, (unsigned)v.x, (unsigned)v.y, cl1);
  }
}

void draw_parametric_curve_uniform_space(image* img, void (*draw)(image*, unsigned, unsigned, void*), vec2 (*f)(float t, void* cl), float t0, float t1, float s0, float spacing, float tolerance, void* cl1, void* cl2) {
  assert(tolerance > 0 && tolerance < 1);
  vec2 last;
  vec2 next = f(t0, cl2);
  
  //The distance must lie between these bounds for a point to be accepted.
  float minDist = (spacing * tolerance);
  float maxDist = (spacing / tolerance);
  
  float increment = s0;  
  while(t0 <= t1) {
    draw(img, (unsigned)last.x, (unsigned) last.y, cl1);
    unsigned trials = 0;
    float d;
    do {
      last = next;
      next = f(t0 + increment, cl2);
      d = vDistance(last, next);
      trials++;
      increment *= (trials * .01 + spacing) / (trials * .01 + d) + EPSILON;
      //printf("%f %f %f %f %u\n", t0, t1, increment, d, trials);
    } while(trials < 20 && (d < minDist || d > maxDist)); //TODO pick constants more intelligently, or base on pixels.
    t0 += increment;
  }
  
  //last = f(t1, cl2);
  //draw(img, (unsigned)last.x, (unsigned) last.y, cl1);
}

//TODO the next thing to do is to test this function and parametric curve 2.
//Then work on using the derivatives of these curves as force vectors.


vec2 parametric_curve_1(float t, void* cl) {
  ccl_1* ccl = cl;
  
  float theta = t;
  float r = sinf(theta * ccl->frequency) * ccl->amplitude + ccl->base;
  
  vec2 result = { cosf(theta + ccl->theta0) * r + ccl->x0, sinf(theta + ccl->theta0) * r + ccl->y0 };
  
  return result;
}

vec2 parametric_curve_2(float t, void* cl) {
  ccl_2* ccl = cl;
  
  vec2 result = {0, 0};
  for(unsigned i = 0; i < PC2_SUMMANDS / 2; i++) {
    result.x += expf(-ccl->d[i] * t) * sinf(t * ccl->f[i] + ccl->p[i]);
    unsigned j = PC2_SUMMANDS / 2 + i;
    result.y += expf(-ccl->d[j] * t) * sinf(t * ccl->f[j] + ccl->p[j]);
  }
  
  vec2 offset = {ccl->x0, ccl->y0};
  
  return vPlus(vRotate(vScale(ccl->scale, result), ccl->theta), offset);
}


void randomize_ccl_1(ccl_1* cl, float x0, float y0, float scale) {
  ccl_1* ccl = cl;
  ccl->x0 = x0;
  ccl->y0 = y0;
  ccl->theta0 = uniformFloat(0, TAU);
  ccl->base = uniformFloat(0, scale);
  ccl->amplitude = uniformFloat(0, scale);
  ccl->frequency = uniformInt(1, 64);
}

void randomize_ccl_2(ccl_2* ccl, float x0, float y0, float scale) {
  ccl->x0 = x0;
  ccl->y0 = y0;
  ccl->scale = scale * 2 / PC2_SUMMANDS;
  ccl->theta = uniformFloat(0, TAU);
  float d = 1;
  for(unsigned i = 0; i < PC2_SUMMANDS; i++) {
    d *= uniformFloat(EPSILON, 1);
    ccl->d[i] = d;
    ccl->f[i] = uniformFloat(0, 1);
    ccl->p[i] = uniformFloat(0, TAU);
  }
}

