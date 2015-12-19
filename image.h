
#ifndef IMAGE_H
#define IMAGE_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define R 0
#define G 1
#define B 2
#define C 3

#define ADJACENT 1
#define SEPARATE 2
#define PIXMODE SEPARATE
#define IMG_EPSILON 0.000001

//TODO pointer is unnecessary with in place data storage.
typedef struct image {
  unsigned width, height;
  float data[];
} image;

image* image_new(unsigned width, unsigned height);

void image_free(image* i);


float* image_pixel(image* i, unsigned x, unsigned y, unsigned c);

static float clampf(float f, float f0, float f1) {
  assert(f0 <= f1);
  if(f < f0) {
    return f0;
  } else if (f > f1) {
    return f1;
  }
  return f;
}

static uint8_t clampf8(float f) {
  if(f >= 1) return (uint8_t) 0xff;
  else if (f < 0) return 0;
  else return (uint8_t) (0xff * f);
}

static uint16_t clampf16(float f) {
  if(f >= 1) return (uint16_t) 0xffff;
  else if (f < 0) return 0;
  else return (uint8_t) (0xffff * f);
}

uint8_t image_pixel_8bit(image* i, unsigned x, unsigned y, unsigned c, uint8_t cmax);
uint16_t image_pixel_16bit(image* i, unsigned x, unsigned y, unsigned c, uint16_t cmax);

uint8_t image_pixel_8bit_full(image* i, unsigned x, unsigned y, unsigned c);
uint16_t image_pixel_16bit_full(image* i, unsigned x, unsigned y, unsigned c);

void image_write_ppm(image* i, FILE* f, uint16_t colors);
void image_print(image* i, FILE* f);

#endif
