
#include "draw.h"
#include "image.h"
#include "noise.h"

void fill_rect(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float r, float g, float b) {
  assert(x1 <= i->width);
  assert(y1 <= i->height);
  for(unsigned y = y0; y < y1; y++) {
    for(unsigned x = x0; x < x1; x++) {
      *image_pixel(i, x, y, R) = r;
      *image_pixel(i, x, y, G) = g;
      *image_pixel(i, x, y, B) = b;
    }
  }
}

void fill_image(image* i, float r, float g, float b) {
  for(unsigned p = 0; p < i->width * i->height; p++) {
    i->data[p * C + R] = r;
    i->data[p * C + G] = g;
    i->data[p * C + B] = b;
  }
}

//Fill a rectangle, colors chosen using the provided lambda.
void fill_rect_f_bw(image* i, unsigned x0, unsigned y0, unsigned x1, unsigned y1, float (*f)(unsigned x, unsigned y, void* cl), void* cl) {
  assert(x1 <= i->width);
  assert(y1 <= i->height);
  for(unsigned y = y0; y < y1; y++) {
    for(unsigned x = x0; x < x1; x++) {
      *image_pixel(i, x, y, R) = *image_pixel(i, x, y, G) = *image_pixel(i, x, y, B) = f(x, y, cl);
    }
  }
}

float noisePaint(unsigned x, unsigned y, void* cl) {
  return noise_sum_2d(x, y, cl);
}
#include <stdio.h>
void draw_point(image* img, unsigned x, unsigned y, void* cl) {
  color* col = cl;
  if(x < img->width && y < img->width) {
    for(unsigned c = 0; c < C; c++) {
      float* p = image_pixel(img, x, y, c);
      *p = *p * (1 - col->a) + col->c[c] * col->a;
    }
  }
}

void draw_circle(image* img, unsigned x, unsigned y, void* cl) {
  color* col = cl;
  float r = ((circle_cl*)cl)->r;
  int ru = (int)r;
  unsigned rSqr = (unsigned)(r * r);
  
  for(int dx = -ru; dx <= ru; dx++) {
    for(int dy = -ru; dy <= ru; dy++) {
      unsigned rrSqr = dx * dx + dy * dy;
      if(rrSqr <= rSqr) {
        draw_point(img, x + dx, y + dy, col);
      }
    }
  }
}

