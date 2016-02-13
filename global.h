#ifndef GLOBAL_H
#define GLOBAL_H

//This module contains many miscellaneous items, mostly math.
//Either they don't fit anywhere else or are so fundamental so as to be useful almost everwhere.
//We also include useful types, including sized integer types and booleans.

#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

/////////////////////////
//Mathematical Constants:

#define PI 3.14159627
#define TAU (2 * PI)

//Small values to use for many things.
//Tiny epsilon used for inexact testing.
#define EPSILON (1.0 / (1ull << 16ull))
//"Differential" epsilon, used for calculus.
#define DEPSILON (1.0 / (1ull << 12ull))

/////////
//Macros:

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

////////////////////
//Basic Vector Math:

typedef struct vec2 {
  float x, y;
} vec2;

typedef struct vec2i {
  int x, y;
} vec2i;

vec2 vPlus(vec2 a, vec2 b);
vec2 vMinus(vec2 a, vec2 b);
vec2 vScale(float s, vec2 a);
float vDot(vec2 a, vec2 b);

float vDSqr(vec2 a, vec2 b);
float vDistance(vec2 a, vec2 b);

vec2 vNormalize(vec2 a);

int vEqual(vec2 a, vec2 b);

vec2 vRotate(vec2 a, float theta);

bool bounded(float f, float bottom, float top);
bool bounded01(float f);

//Rounding
//Adapted from "http://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int"
//Could use "https://gist.github.com/orlp/3551590" for a more optimized version if high exponents are common.
unsigned upow(unsigned base, unsigned exp);
unsigned uroundf(float f);
unsigned uceilf(float f);

//Clamp floats
float clampf(float f, float f0, float f1);
uint8_t clampf8(float f);
uint16_t clampf16(float f);

#endif
