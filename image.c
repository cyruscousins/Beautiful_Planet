
#include "image.h"

image* image_new(unsigned width, unsigned height) {
  image* i = malloc(sizeof(image) + width * height * C * sizeof(float));
  i->width = width;
  i->height = height;
  return i;
}

void image_free(image* i) {
  free(i);
}

float* image_pixel(image* i, unsigned x, unsigned y, unsigned c) {
  assert(x < i->width);
  assert(y < i->height);
  assert(c < C);
  
  #if (PIXMODE == ADJACENT)
  //Pixels stored as triplets.
  return i->data + (y * i->width + x) * C + c;
  #elif (PIXMODE == SEPARATE)
  //Pixels stored with separate RGB channels.
  return i->data + (i->width * i->width * c) + (y * i->width + x);
  #else
  //Error
  assert(0);
  #endif
}

uint8_t image_pixel_8bit(image* i, unsigned x, unsigned y, unsigned c, uint8_t cmax) {
  float f = clamp(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint8_t) (f * (((float)cmax) + 1)); 
}

uint16_t image_pixel_16bit(image* i, unsigned x, unsigned y, unsigned c, uint16_t cmax) {
  float f = clamp(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint16_t) (f * (((float)cmax) + 1)); 
}


//TODO nonlinear gamma correction.
void image_write_ppm(image* i, FILE* f, uint16_t depth) {
  fprintf(f, "P6\n%u %u %u\n", i->width, i->height, depth);
  for(unsigned y = 0; y < i->height; y++) {
    for(unsigned x = 0; x < i->width; x++) {
      if(depth < 256) {
        uint8_t r = image_pixel_8bit(i, x, y, R, (uint8_t)depth);
        uint8_t g = image_pixel_8bit(i, x, y, G, (uint8_t)depth);
        uint8_t b = image_pixel_8bit(i, x, y, B, (uint8_t)depth);
        fprintf(f, "%c%c%c", r, g, b);
      } else {
        uint16_t r = image_pixel_16bit(i, x, y, R, (uint16_t)depth);
        uint16_t g = image_pixel_16bit(i, x, y, G, (uint16_t)depth);
        uint16_t b = image_pixel_16bit(i, x, y, B, (uint16_t)depth);
        uint8_t rh = (uint8_t)((r & 0xff00) >> 8);
        uint8_t rl = (uint8_t)(r & 0xff);
        uint8_t gh = (uint8_t)((g & 0xff00) >> 8);
        uint8_t gl = (uint8_t)(g & 0xff);
        uint8_t bh = (uint8_t)((b & 0xff00) >> 8);
        uint8_t bl = (uint8_t)(b & 0xff);
        fprintf(f, "%c%c%c%c%c%c", rh, rl, gh, gl, bh, bl);
      }
    }
  }
}

void image_print(image* i, FILE* f) {
  fprintf(f, "{IMAGE %d x %d\n", i->width, i->height);
  for(unsigned y = 0; y < i->height; y++) {
    for(unsigned x = 0; x < i->width; x++) {
      fprintf(f, "%.3f\t%.3f\t%.3f\t\t", *image_pixel(i, x, y, R), *image_pixel(i, x, y, G), *image_pixel(i, x, y, B));
    }
    fprintf(f, "\n");
  }
  fprintf(f, "}\n");
}
