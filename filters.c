
#include <string.h>

#include "filters.h"
#include "convolution.h"

//A draft blur.  Doesn't handle the borders well, and only handles square images.
image* image_blur(image* i, unsigned br) {
  assert(i->width == i->height);

  unsigned bw = br * 2 + 1;
  float kernel[bw * bw];
  round_blur_kernel_2d(kernel, br); 
  image* i2 = image_new(i->width, i->height);
  for(unsigned c = 0; c < C; c++) {
    convolve_kernel_square_2d(image_pixel(i, 0, 0, c), image_pixel(i2, 0, 0, c), i->width, kernel, br);
  }
  
  return i2;
}

void image_blur_inplace(image* i, unsigned br) {
  assert(i->width == i->height);
  
  float* cpy = malloc(sizeof(float) * i->width * i->height);

  unsigned bw = br * 2 + 1;
  float kernel[bw * bw];
  round_blur_kernel_2d(kernel, br); 
  for(unsigned c = 0; c < C; c++) {
    float* cChannel = image_pixel(i, 0, 0, c);
    memcpy(cpy, cChannel, i->width * i->height * sizeof(float));
    convolve_kernel_square_2d(cpy, cChannel, i->width, kernel, br);
  }
  
  free(cpy);
}

float avgNeighbors(float* f, unsigned x, unsigned y, unsigned w, unsigned h) {
  return (             f[(y - 1) * w + x] +
    f[y * w + x - 1] + f[y * w + x]       + f[y * w + x + 1] +
                       f[(y + 1) * w + x]
    ) / 5;
}

float avgNeighborsEdge(float* f, unsigned x, unsigned y, unsigned w, unsigned h) {
  float result = f[y * w + x];
  unsigned count = 1;
  if(x > 1) {
    result += f[y * w + x - 1];
    count++;
  }
  if(x < w - 1) {
    result += f[y * w + x + 1];
    count++;
  }
  if(y > 1) {
    result += f[(y - 1) * w + x];
    count++;
  }
  if(y < h - 1) {
    result += f[(y + 1) * w + x];
    count++;
  }
  return result / count;
}


//Fast blur technique suited to video if it is called repeatedly.  Essentially splits the work of a blur over time.
void image_blur_fast_inplace(image* i, unsigned br) {
  unsigned w = i->width;
  unsigned h = i->height;
  unsigned x0, x1, y0, y1, o, oo;
  
  /*
  if(br % 2 == 0) {
    x0 = 1;
    x1 = w / 2;
  } else {
    x0 = w / 2;
    x1 = w - 1;
  }
  if(br / 2 % 2 == 0) {
    y0 = 1;
    y1 = i->height / 2;
  } else {
    y0 = i->height / 2;
    y1 = i->height - 1;
  }
  
  o = br / 4 % 2;
  */
  
  x0 = y0 = 1;
  x1 = i->width - 1;
  y1 = i->height - 1;
  oo = br % 2;
  o = oo;
  
  float* ch[3] = {
    image_pixel(i, 0, 0, R), image_pixel(i, 0, 0, G), image_pixel(i, 0, 0, B)
  };
  
  for(unsigned c = 0; c < C; c++) {
    //TODO could phase color channels.
    o = !o;
    for(unsigned y = y0; y < y1; y++) {
      for(unsigned x = x0 + o; x < x1; x += 2) {
        ch[c][y * w + x] = avgNeighbors(ch[c], x, y, w, h);
      }
    }
    
    //Edges.
    for(unsigned x = oo; x < w; x += 2) {
      ch[c][x] = avgNeighborsEdge(ch[c], x, 0, w, h);
    }
    for(unsigned y = oo; y < h; y += 2) {
      ch[c][y * w] = avgNeighborsEdge(ch[c], 0, y, w, h);
    }
    
    for(unsigned x = (oo + h) % 2; x < w; x += 2) {
      ch[c][(h - 1) * w + x] = avgNeighborsEdge(ch[c], x, h - 1, w, h);
    }
    for(unsigned y = (oo + w) % 2; y < h; y += 2) {
      ch[c][y * w + w - 1] = avgNeighborsEdge(ch[c], w - 1, y, w, h);
    }
  }
}

