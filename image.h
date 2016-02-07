
#ifndef IMAGE_H
#define IMAGE_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "global.h"

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

uint8_t image_pixel_8bit(image* i, unsigned x, unsigned y, unsigned c, uint8_t cmax);
uint16_t image_pixel_16bit(image* i, unsigned x, unsigned y, unsigned c, uint16_t cmax);

uint8_t image_pixel_8bit_full(image* i, unsigned x, unsigned y, unsigned c);
uint16_t image_pixel_16bit_full(image* i, unsigned x, unsigned y, unsigned c);

void image_write_ppm(image* i, FILE* f, uint16_t colors);
void image_print(image* i, FILE* f);

void image_draw(image* d, image* i, vec2i dp, vec2i sp, unsigned w, unsigned h);


void image_draw(image* d, image* s, vec2i dp, vec2i sp, unsigned w, unsigned h);
//This function draws s to d with the given offsets, only drawing to the intersection of the shifted images.
void image_draw_checked(image* d, image* s, vec2i dp, vec2i sp, unsigned w, unsigned h);

//TODO image drawing with alpha channel.

#endif
