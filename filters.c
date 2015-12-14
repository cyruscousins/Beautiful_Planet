
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

