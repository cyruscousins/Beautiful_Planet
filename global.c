#include <math.h>

#include "global.h"

//Vector math
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

//Bounding and clamping
bool bounded(float f, float bottom, float top) {
  return f >= bottom && f <= top;
}
bool bounded01(float f) {
  return bounded(f, 0, 1);
}

float clampf(float f, float f0, float f1) {
  assert(f0 <= f1);
  if(f < f0) {
    return f0;
  } else if (f > f1) {
    return f1;
  }
  return f;
}

uint8_t clampf8(float f) {
  if(f >= 1) return (uint8_t) 0xff;
  else if (f < 0) return 0;
  else return (uint8_t) (0xff * f);
}

uint16_t clampf16(float f) {
  if(f >= 1) return (uint16_t) 0xffff;
  else if (f < 0) return 0;
  else return (uint8_t) (0xffff * f);
}

//Basic
//Adapted from "http://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int".
//Could use "https://gist.github.com/orlp/3551590" for a more optimized version if high exponents are common.
unsigned upow(unsigned base, unsigned exp) {
  unsigned result = 1;
  while (exp) {
    if (exp & 1) result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
}

unsigned uroundf(float f) {
  assert(f >= -0.5);
  return (unsigned)(f + 0.5);
}
unsigned uceilf(float f) {
  assert(f >= 0);
  return (unsigned)(f + 1.0 - EPSILON);
}
