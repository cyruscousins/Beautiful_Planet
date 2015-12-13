
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

//TODO pointer is unnecessary with in place data storage.
typedef struct image {
  float* data;
  unsigned width, height;
} image;

static image* image_new(unsigned width, unsigned height) {
  image* i = malloc(sizeof(image) + width * height * C * sizeof(float));
  i->data = (float*)(i + 1);
  i->width = width;
  i->height = height;
  return i;
}

static void image_free(image* i) {
  free(i);
}

static float* image_pixel(image* i, unsigned x, unsigned y, unsigned c) {
  assert(x < i->width);
  assert(y < i->height);
  assert(c < C);
  return i->data + (y * i->width + x) * C + c;
}

static double clamp(float f, float f0, float f1) {
  assert(f0 <= f1);
  if(f < f0) {
    return f0;
  } else if (f > f1) {
    return f1;
  }
  return f;
}

#define IMG_EPSILON 0.000001
static uint8_t image_pixel_8bit(image* i, unsigned x, unsigned y, unsigned c, uint8_t cmax) {
  float f = clamp(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint8_t) (f * (((float)cmax) + 1)); 
}

static uint8_t image_pixel_16bit(image* i, unsigned x, unsigned y, unsigned c, uint16_t cmax) {
  float f = clamp(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint16_t) (f * (((float)cmax) + 1)); 
}

void image_write_ppm(image* i, FILE* f, uint16_t colors);

void image_print(image* i, FILE* f);
#endif
