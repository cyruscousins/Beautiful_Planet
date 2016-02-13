
#ifndef DRAW_H
#define DRAW_H

#include "image.h"

//Colors and drawing structs:
typedef struct color {
  float c[C];
  float a;
} color;

typedef struct circle_cl {
  color c;
  float r;
} circle_cl;

typedef struct random_cl {
  unsigned count, range;
  color* col;
} random_cl;

typedef void (*drawfun)(image*, unsigned, unsigned, void*);

void randomize_color(color* c, float cLow, float cHigh, float alpha);

//Drawing rectangles
void fill_rect(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float r, float g, float b);

//Drawing to the entire image
void image_fill_channel(image* i, unsigned c, float f);
void fill_image(image* i, float r, float g, float b);
void fill_image_a(image* i, float r, float g, float b, float a);

void image_multiply_channel(image* i, unsigned c, float f);
void image_multiply(image* i, float f);

void fill_rect_f_bw(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float (*f)(unsigned x, unsigned y, void* cl), void* cl);

float noisePaint(unsigned x, unsigned y, void* cl);

void draw_point_additive(image* img, unsigned x, unsigned y, void* cl); //closure is a color with no alpha channel.
void draw_point(image* img, unsigned x, unsigned y, void* cl); //closure is a color
void draw_circle(image* img, unsigned x, unsigned y, void* cl); //closure is a circle_cl

void draw_random(image* img, unsigned x, unsigned y, void* cl); //closure is a random_cl
void draw_random_additive(image* img, unsigned x, unsigned y, void* cl); //Closure is a random_cl

#endif

