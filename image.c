
#include "image.h"
#include "global.h"

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
  float f = clampf(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint8_t) (f * (((float)cmax) + 1)); 
}

uint16_t image_pixel_16bit(image* i, unsigned x, unsigned y, unsigned c, uint16_t cmax) {
  float f = clampf(*image_pixel(i, x, y, c), 0, 1 - IMG_EPSILON);
  return (uint16_t) (f * (((float)cmax) + 1)); 
}

uint8_t image_pixel_8bit_full(image* i, unsigned x, unsigned y, unsigned c) {
  return clampf8(*image_pixel(i, x, y, c)); 
}

uint16_t image_pixel_16bit_full(image* i, unsigned x, unsigned y, unsigned c) {
  return clampf16(*image_pixel(i, x, y, c)); 
}

//TODO nonlinear gamma correction.
void image_write_ppm(image* i, FILE* f, uint16_t depth) {
  fprintf(f, "P6\n%u %u %u\n", i->width, i->height, depth);
  if(depth == 255) {
    //Fast path for 8 bit color.
    char* data = malloc(3 * i->height * i->width);
    for(unsigned y = 0; y < i->height; y++) {
      for(unsigned x = 0; x < i->width; x++) {
        uint8_t r = image_pixel_8bit_full(i, x, y, R);
        uint8_t g = image_pixel_8bit_full(i, x, y, G);
        uint8_t b = image_pixel_8bit_full(i, x, y, B);
        data[3 * (y * i->width + x) + 0] = r;
        data[3 * (y * i->width + x) + 1] = g;
        data[3 * (y * i->width + x) + 2] = b;
      }
    }
    fwrite(data, 3 * i->height * i->width, 1, f);
    free(data);
  } else if(depth < 256) {
    //Fast path for low bitrate images.
    char* data = malloc(3 * i->height * i->width);
    for(unsigned y = 0; y < i->height; y++) {
      for(unsigned x = 0; x < i->width; x++) {
        uint8_t r = image_pixel_8bit(i, x, y, R, (uint8_t)depth);
        uint8_t g = image_pixel_8bit(i, x, y, G, (uint8_t)depth);
        uint8_t b = image_pixel_8bit(i, x, y, B, (uint8_t)depth);
        data[3 * (y * i->width + x) + 0] = r;
        data[3 * (y * i->width + x) + 1] = g;
        data[3 * (y * i->width + x) + 2] = b;
      }
    }
    fwrite(data, 3 * i->height * i->width, 1, f);
    free(data);
  } else {
    for(unsigned y = 0; y < i->height; y++) {
      for(unsigned x = 0; x < i->width; x++) {
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

void image_draw(image* d, image* s, vec2i dp, vec2i sp, unsigned w, unsigned h) {
  assert(dp.x >= 0);
  assert(dp.y >= 0);
  
  assert(sp.x >= 0);
  assert(sp.y >= 0);
  
  assert(dp.x + w <= d->width);
  assert(dp.y + h <= d->height);
  
  assert(sp.x + w <= s->width);
  assert(sp.y + h <= s->height);
  
  for(unsigned y = 0; y < h; y++) {
    for(unsigned x = 0; x < w; x++) {
      //TODO cache-efficient version?
      for(unsigned c = 0; c < C; c++) {
        *image_pixel(d, dp.x + x, dp.y + y, c) = *image_pixel(s, sp.x + x, sp.y + y, c);
      }
    }
  }
}

//This function draws s to d with the given offsets, only drawing to the intersection of the shifted images.
void image_draw_checked(image* d, image* s, vec2i dp, vec2i sp, unsigned w, unsigned h) {
  int minx = MIN(dp.x, sp.x);
  int miny = MIN(dp.y, sp.y);
  
  if(minx < 0) {
    sp.x -= minx;
    dp.x -= minx;
    w += minx;
  }
  if(miny < 0) {
    sp.y -= miny;
    dp.y -= miny;
    h += miny;
  }
  
  int maxxo = MIN((int)d->width - (dp.x + (int)w), (int)s->width - (sp.x + (int)w));
  int maxyo = MIN((int)d->height - (dp.y + (int)h), (int)s->height - (sp.y + (int)h));
    
  if(maxxo < 0) {
    w += maxxo;
  }
  if(maxyo < 0) {
    h += maxyo;
  }
  
  if((int)w <= 0 || (int)h <= 0) {
    return; //No intersection.
  }
  
  image_draw(d, s, dp, sp, w, h);
}

