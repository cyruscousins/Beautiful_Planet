
#ifndef ART_H
#define ART_H

#include "image.h"

typedef void (*image_processor)(image* img, void* cl);

void gravity(image_processor p, unsigned imageWidth, unsigned imageHeight, unsigned frames, unsigned nsCount, unsigned nsDepth);
void curves(image_processor p, unsigned imageWidth, unsigned imageHeight, unsigned frames);
void dust(image_processor p, unsigned imageWidth, unsigned imageHeight, unsigned frames, unsigned nsCount, unsigned nsDepth);

#endif
