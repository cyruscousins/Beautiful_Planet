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

void randomize_hypocycloid(cycloid* ccl, float x0, float y0, float scale) {
  ccl->x0 = x0;
  ccl->y0 = y0;
  ccl->theta0 = uniformFloat(0, TAU);
  
  ccl->rb = scale;
  ccl->r = scale / uniformInt(2, 17);
  ccl->a = uniformFloat(0, 4 * ccl->r);
}

void randomize_epicycloid(cycloid* ccl, float x0, float y0, float scale) {
  randomize_hypocycloid(ccl, x0, y0, scale);
}

vec2 hypocycloid(float t, void* ccl) {
  cycloid* cl = ccl;
  vec2 result = {(cl->rb - cl->r) * cosf(cl->r / cl->rb * t) + cl->a * cosf((1 - cl->r / cl->rb) * t), (cl->rb - cl->r) * sinf(cl->r / cl-> rb * t) - cl->a * sinf((1 - cl->r / cl->rb) * t)};
  
  vec2 offset = {cl->x0, cl->y0};
  
  return vPlus(vRotate(result, cl->theta0), offset);
}
vec2 epicycloid(float t, void* ccl) {
  cycloid* cl = ccl;
  vec2 result = {(cl->rb + cl->r) * cosf(cl->r / cl->rb * t) - cl->a * cosf((1 + cl->r / cl->rb) * t), (cl->rb + cl->r) * sinf(cl->r / cl-> rb * t) - cl->a * sinf((1 + cl->r / cl->rb) * t)};
  
  vec2 offset = {cl->x0, cl->y0};
  
  return vPlus(vRotate(result, cl->theta0), offset);
}

vec2 parametric_curve_weighted_sum(float t, void* cl) {
  weighted_sum_pcl* pcl = cl;
  vec2 result = {0, 0};
  for(unsigned i = 0; i < pcl->count; i++) {
    float tt = t * pcl->timeFactors[i] + pcl->timeSummands[i];
    vec2 vv = pcl->f[i](tt, pcl->cl[i]);
    result = vPlus(result, vScale(pcl->weights[i], vv));
  }
  return result;
}

vec2 parametric_curve_point(float t, void* cl) {
  vec2* vcl = cl;
  return *vcl;
}

weighted_sum_pcl* randomize_weighted_sum(float x0, float y0, float scale, unsigned maxComponents) {
  assert(maxComponents > 1);
  unsigned count = uniformInt(2, 1 + 1 + maxComponents);
  
  weighted_sum_pcl* w = malloc(sizeof(weighted_sum_pcl));
  w->count = count;
  
  w->cl = malloc(sizeof(void*) * count);
  w->f = malloc(sizeof(void*) * count);
  w->weights = malloc(sizeof(float) * count);
  w->timeFactors = malloc(sizeof(float) * count);
  w->timeSummands = malloc(sizeof(float) * count);
  
  vec2 p = {x0, y0};
  w->cl[0] = malloc(sizeof(vec2));
  *((vec2*)w->cl[0]) = p;
  w->f[0] = parametric_curve_point;
  w->weights[0] = 1;
  w->timeFactors[0] = w->timeSummands[0] = 0;
  
  for(unsigned i = 1; i < count; i++) {
    unsigned type = uniformInt(0, 4);
    
    w->weights[i] = uniformFloat(1.0 / 2.0, 2);
    w->timeFactors[i] = uniformFloat(1.0 / 2.0, 2);
    w->timeSummands[i] = uniformFloat(0, TAU);
    
    switch(type) {
      case 0:
        w->cl[i] = malloc(sizeof(ccl_1));
        w->f[i] = parametric_curve_1;
        randomize_ccl_1(w->cl[i], 0, 0, scale / count);
        break;
      case 1:
        w->cl[i] = malloc(sizeof(ccl_2));
        w->f[i] = parametric_curve_2;
        randomize_ccl_2(w->cl[i], 0, 0, scale / count);
        w->timeFactors[i] = 1;
        break;
      case 2:
        w->cl[i] = malloc(sizeof(cycloid));
        w->f[i] = hypocycloid;
        randomize_hypocycloid(w->cl[i], 0, 0, scale / count);
        break;
      case 3:
        w->cl[i] = malloc(sizeof(cycloid));
        w->f[i] = epicycloid;
        randomize_epicycloid(w->cl[i], 0, 0, scale / count);
        break;
      default:
        assert(0);
        break;
    }
  }
  return w;
}

void free_weighted_sum(weighted_sum_pcl* w) {
  for(unsigned i = 0; i < w->count; i++) {
    free(w->cl[i]);
  }
  free(w->cl);
  free(w->f);
  free(w->timeFactors);
  free(w->timeSummands);
}


//Set up the function tables

typedef vec2 (*wsps_perturb_f)(void* cl, float amt);
typedef vec2 (*wsps_randomize_f)(void* cl, float x0, float y0, float scale);

pc2d_f wsps_f_table[ws_count] = {
  parametric_curve_1,
  parametric_curve_2,
  hypocycloid,
  epicycloid,
};

wsps_perturb_f wsps_perturb_table[ws_count] = {
  NULL, NULL, NULL, NULL //TODO
};

unsigned wsps_cl_sizes[ws_count] = {
  sizeof(ccl_1), sizeof(ccl_2), sizeof(cycloid), sizeof(cycloid)
};

wsps_randomize_f wsps_randomize_table[ws_count] = {
  //NULL, NULL, NULL, NULL //TODO
  (wsps_randomize_f) randomize_ccl_1, (wsps_randomize_f) randomize_ccl_2, (wsps_randomize_f) randomize_hypocycloid, (wsps_randomize_f) randomize_epicycloid
};

vec2 parametric_curve_weighted_static_sum(float t, void* cl) {
  weighted_sum_pcl_static* pcl = cl;
  vec2 result = {0, 0};
  for(unsigned i = 0; i < pcl->count; i++) {
    assert(pcl->types[i] < ws_count);
    float tt = t * pcl->timeFactors[i] + pcl->timeSummands[i];
    vec2 vv = wsps_f_table[(unsigned)pcl->types[i]](tt, pcl->cl[i]);
    result = vPlus(result, vScale(pcl->weights[i], vv));
  }
  
  vec2 offset = {pcl->x0, pcl->y0};
  
  return vPlus(vRotate(result, pcl->theta), offset);
}
void perturb_weighted_static_sum(weighted_sum_pcl_static* w, float amt) {
  //Perturb rotation and position
  w->x0 += uniformFloatS(amt);
  w->y0 += uniformFloatS(amt);
  w->theta += uniformFloatS(amt / 4.0);

  for(unsigned i = 0; i < w->count; i++) {
    //Perturb weights
    float perturbationAmt = uniformFloat(1.0 - amt * 4, 1.0);
    if(uniformInt(0, 2)) {
      perturbationAmt = 1.0 / perturbationAmt;
    }
    w->weights[i] *= perturbationAmt;
    
    w->timeSummands[i] += uniformFloatS(amt);
    //Perturb curves
    switch(w->types[i]) {
      case ws_c1:
        perturbationAmt = uniformFloat(1.0 - amt, 1.0);
        if(uniformInt(0, 2)) {
          perturbationAmt = 1.0 / perturbationAmt;
        }
        ((ccl_1*)w->cl[i])->amplitude *= perturbationAmt;
        perturbationAmt = uniformFloat(1.0 - amt, 1.0);
        if(uniformInt(0, 2)) {
          perturbationAmt = 1.0 / perturbationAmt;
        }
        ((ccl_1*)w->cl[i])->base *= perturbationAmt;
        break;
      case ws_c2:
        perturbationAmt = uniformFloat(1.0 - amt, 1.0);
        if(uniformInt(0, 2)) {
          perturbationAmt = 1.0 / perturbationAmt;
        }
        //Change a d or an f (multiplicatively).
        unsigned valToChange = uniformInt(0, 8);
        ((ccl_2*)w->cl[i])->d[valToChange] *= perturbationAmt;
        //Change a p (additivitively).
        valToChange = uniformInt(0, 4);
        ((ccl_2*)w->cl[i])->p[valToChange] += uniformFloatS(amt);
        break;
      case ws_hypocycloid:
      case ws_epicycloid:
        perturbationAmt = uniformFloat(1.0 - amt, 1.0);
        if(uniformInt(0, 2)) {
          perturbationAmt = 1.0 / perturbationAmt;
        }
        ((cycloid*)w->cl[i])->rb *= perturbationAmt;
        ((cycloid*)w->cl[i])->r *= perturbationAmt;
        ((cycloid*)w->cl[i])->a += uniformFloatS(amt);
        break;
    }
  }
}

weighted_sum_pcl_static* randomize_weighted_static_sum(float x0, float y0, float scale, unsigned maxComponents) {
  assert(maxComponents > 0);
  weighted_sum_pcl_static* pcl = malloc(sizeof(weighted_sum_pcl_static));
  unsigned count = uniformInt(1, maxComponents + 1);

  pcl->x0 = x0;
  pcl->y0 = y0;
  pcl->count = count;
  pcl->theta = uniformFloat(0, TAU);
  
  pcl->types = malloc(sizeof(char) * count);
  pcl->cl = malloc(sizeof(void*) * count);
  pcl->weights = malloc(sizeof(float) * count * 3);
  pcl->timeFactors = pcl->weights + count;
  pcl->timeSummands = pcl->timeFactors + count;
  
  for(unsigned i = 0; i < count; i++) {
    unsigned type = uniformInt(0, ws_count);
    pcl->types[i] = (char) type;
    pcl->cl[i] = malloc(wsps_cl_sizes[type]);
    wsps_randomize_table[(unsigned)pcl->types[i]](pcl->cl[i], 0, 0, scale / count);
    pcl->weights[i] = 1.0 / count;
    pcl->timeFactors[i] = uniformFloat(1.0 / 2.0, 2);
    pcl->timeSummands[i] = (type == ws_c2) ? 0 : uniformFloat(0, TAU);
  }
  return pcl;
}

