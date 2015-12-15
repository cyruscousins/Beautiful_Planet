
#ifndef DRAW_H
#define DRAW_H

#include "image.h"

typedef struct color {
  float c[C];
  float a;
} color;

typedef struct circle_cl {
  color c;
  float r;
} circle_cl;

void randomize_color(color* c, float cLow, float cHigh, float alpha);

void fill_rect(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float r, float g, float b);
void fill_image(image* i, float r, float g, float b);
void fill_image_a(image* i, float r, float g, float b, float a);

void fill_rect_f_bw(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float (*f)(unsigned x, unsigned y, void* cl), void* cl);

float noisePaint(unsigned x, unsigned y, void* cl);

void draw_point_additive(image* img, unsigned x, unsigned y, void* cl); //closure is a color with no alpha channel.
void draw_point(image* img, unsigned x, unsigned y, void* cl); //closure is a color
void draw_circle(image* img, unsigned x, unsigned y, void* cl); //closure is a circle_cl

#endif

