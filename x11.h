#ifndef X_BINDINGS_H
#define X_BINDINGS_H

#define INTERACTIVE

#include <stdbool.h>

#include "global.h"
#include "image.h"

void x_window_init();
int x_window_create(unsigned image_width, unsigned image_height);
int x_window_display_image(image* img);
void x_window_cleanup();

#ifdef INTERACTIVE
bool isTerminated();
bool isKeyPressed(char k);
void setKeyPressed(char k, bool pressed);
vec2 getMousePosition();
int getMouseX();
int getMouseY();
void processXEvents();

typedef enum keys {ESCAPE, SPACE, SHIFT} keys_t;
#endif

#endif
