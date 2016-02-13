#ifndef VECTOR_H
#define VECTOR_H

# include "global.h"
# include "draw.h"
# include "image.h"

typedef struct line {
  vec2 start;
  vec2 end;
  color c;
} line_t;

typedef struct chain {
  vec2 *points;
  int num_points;
  color c;
} chain_t;


typedef struct cchain {
  vec2 **points;
  int num_chains;
  int* num_points; // num_points[i] gives num of points in  chain i
  color* c; // c[i] gives color of chain i
} cchain_t;

void draw_line(image* img, unsigned x, unsigned y, void *lp);

void draw_chain(image* img, unsigned x, unsigned y, void *cp);

void draw_cchain(image* img, unsigned x, unsigned y, void *cp);

#endif
