
#ifndef FILTERS_H
#define FILTERS_H

#include "image.h"

image* image_blur(image* i, unsigned br);

void image_blur_inplace(image* i, unsigned br);

void image_blur_fast_inplace(image* i, unsigned offset);

#endif

