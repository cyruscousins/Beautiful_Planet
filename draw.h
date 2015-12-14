
#ifndef DRAW_H
#define DRAW_H

#include "image.h"

void fill_rect(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float r, float g, float b);
void fill_image(image* i, float r, float g, float b);
void fill_image_a(image* i, float r, float g, float b, float a);

void fill_rect_f_bw(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float (*f)(unsigned x, unsigned y, void* cl), void* cl);

float noisePaint(unsigned x, unsigned y, void* cl);

typedef struct color {
  float c[C];
  float a;
} color;

typedef struct circle_cl {
  color c;
  float r;
} circle_cl;

void draw_point(image* img, unsigned x, unsigned y, void* cl); //closure is a color
void draw_circle(image* img, unsigned x, unsigned y, void* cl); //closure is a circle_cl

#endif

