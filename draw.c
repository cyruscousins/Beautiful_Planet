
#include <stdio.h>

#include "draw.h"
#include "image.h"
#include "noise.h"

void randomize_color(color* c, float cLow, float cHigh, float alpha) {
  for(unsigned i = 0; i < C; i++) {
    c->c[i] = uniformFloat(cLow, cHigh);
  }
  c->a = alpha;
}

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


void image_fill_channel(image* i, unsigned c, float f) {
  assert(c < C);
  float* b = image_pixel(i, 0, 0, c);
  for(unsigned p = 0; p < i->width * i->height; p++) {
    b[p] = f;
  }
}

void fill_image(image* i, float r, float g, float b) {
  float colors[3] = {r, g, b};
  for(unsigned c = 0; c < C; c++) {
    image_fill_channel(i, c, colors[c]);
  }
}

void fill_image_a(image* i, float r, float g, float b, float a) {
  float oa = 1 - a;
  r *= a;
  g *= a;
  b *= a;
  float* c;
  c = image_pixel(i, 0, 0, R);
  for(unsigned p = 0; p < i->width * i->height; p++) {
    c[p] = c[p] * oa + r;
  }
  c = image_pixel(i, 0, 0, G);
  for(unsigned p = 0; p < i->width * i->height; p++) {
    c[p] = c[p] * oa + g;
  }
  c = image_pixel(i, 0, 0, B);
  for(unsigned p = 0; p < i->width * i->height; p++) {
    c[p] = c[p] * oa + b;
  }
}

void image_multiply_channel(image* i, unsigned c, float f) {
  assert(c < C);
  float* b = image_pixel(i, 0, 0, c);
  for(unsigned p = 0; p < i->width * i->height; p++) {
    b[p] *= f;
  }
}
void image_multiply(image* i, float f) {
  float* b = image_pixel(i, 0, 0, 0);
  for(unsigned p = 0; p < i->width * i->height * C; p++) {
    b[p] *= f;
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



void draw_point(image* img, unsigned x, unsigned y, void* cl) {
  color* col = cl;
  if(x < img->width && y < img->width) {
    for(unsigned c = 0; c < C; c++) {
      float* p = image_pixel(img, x, y, c);
      *p = *p * (1 - col->a) + col->c[c] * col->a;
    }
  }
}
void draw_point_additive(image* img, unsigned x, unsigned y, void* cl) {
  color* col = cl;
  if(x < img->width && y < img->width) {
    for(unsigned c = 0; c < C; c++) {
      *image_pixel(img, x, y, c) += col->c[c];
    }
  }
}

void draw_random(image* img, unsigned x, unsigned y, void* cl) {
  random_cl* rcl = cl;
  color* col = rcl->col;
  for(unsigned i = 0; i < rcl->count; i++) {
    int dx = uniformInt(-((int)rcl->range), rcl->range + 1);
    int dy = uniformInt(-((int)rcl->range), rcl->range + 1);
    draw_point(img, x + dx, y + dy, col);
  }
}
void draw_random_additive(image* img, unsigned x, unsigned y, void* cl) {
  random_cl* rcl = cl;
  color* col = rcl->col;
  for(unsigned i = 0; i < rcl->count; i++) {
    int dx = uniformInt(-((int)rcl->range), rcl->range + 1);
    int dy = uniformInt(-((int)rcl->range), rcl->range + 1);
    draw_point_additive(img, x + dx, y + dy, col);
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

