#ifndef X_BINDINGS_H
#define X_BINDINGS_H

#include "image.h"

void x_window_init();
int x_window_create(unsigned image_width, unsigned image_height);
int x_window_display_image(image* img);
void x_window_cleanup();


#endif
