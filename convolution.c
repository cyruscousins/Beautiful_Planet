
#include <string.h>
#include <math.h>
#include <stdlib.h>


#include "convolution.h"
#include "assert.h"

#define EPSILON 0.0001

//Calculates the matrix dot product between the submatrix of m1 (a matrix of width w1) centered at (x, y), with m2, a matrix of width "w2".
float calculateMatrixDot(const float* m1, unsigned w1, unsigned x, unsigned y, const float* m2, int w2) {
  assert(w2 <= w1);
  
  float v = 0;
  for(int yo = -w2; yo <= w2; yo++) {
    int yp = ((y + yo) + w1) % w1;
    for(int xo = -w2; xo <= w2; xo++) {
      int xp = ((x + xo) + w1) % w1;
      v += m1[xp + yp * w1] * m2[xo + w2 + (yo + w2) * (w2 * 2 + 1)];
    }
  }
  return v;
}

//Same as above, but no bounds checking / wraparound is performed, so use with caution.
float calculateMatrixDotNoBC(const float* m1, unsigned w1, unsigned x, unsigned y, const float* m2, int w2) {
  assert(x >= w2);
  assert(x < w1 - w2);
  assert(y >= w2);
  assert(y < w1 - w2);
  
  float v = 0;
  for(int yo = -w2; yo <= w2; yo++) {
    int yp = y + yo;
    for(int xo = -w2; xo <= w2; xo++) {
      int xp = x + xo;
      v += m1[xp + yp * w1] * m2[xo + w2 + (yo + w2) * (w2 * 2 + 1)];
    }
  }
  assert(fabs(v - calculateMatrixDot(m1, w1, x, y, m2, w2)) < EPSILON);
  return v;
}

//Unoptimized version with many extra bounds checks
void convolve_kernel_square_2d_slow(const float* fin, float* fout, unsigned w1, const float* convolution, unsigned w2) {
  for(unsigned y = 0; y < w1; y++) {
    for(unsigned x = 0; x < w1; x++) {
      fout[y * w1 + x] = calculateMatrixDot(fin, w1, x, y, convolution, w2);
    }
  }
}

//Apply a convolution kernel to f1, a matrix of width w1.  Kernel, of radius w2, givn by convolution.  Result stored in fout, a matrix of width w1.
void convolve_kernel_square_2d(const float* fin, float* fout, unsigned w1, const float* convolution, unsigned w2) {
  //If the convolution kernel greater than or equal in size to the input, then handle it all with bounds checking.
  if(w2 * 2 + 1 >= w1) {
    convolve_kernel_square_2d_slow(fin, fout, w1, convolution, w2);
    return;
  }
  
  //Otherwise, use this efficient version:
  //Apply a convolution kernel to the interior.
  for(unsigned y = w2; y < w1 - w2; y++) {
    for(unsigned x = w2; x < w1 - w2; x++) {
      fout[y * w1 + x] = calculateMatrixDotNoBC(fin, w1, x, y, convolution, w2);
    }
  }
  
  //Handle the edges.
  for(unsigned y = 0; y < w1; y++) {
    //TODO use a bool.
    char yBorder = y < w2 || y >= w1 - w2; //We're on a yborder, so don't skip the interior x.
    for(unsigned x = 0; x < w1; x++) {
      if(!yBorder && x == w2) {
        x = w1 - w2 - 1;
        continue;
      }
      //TODO this is still somewhat inefficient: we know what bounds need to be checked, this checks them all.  We should instead have 8 double loops (one for each corner).
      fout[y * w1 + x] = calculateMatrixDot(fin, w1, x, y, convolution, w2);
    }
  }
  
  #ifdef DEBUG
  //Compare to a simpler but bitwise identical convolution algorithm.  Epsilon comparison used to guard against unpredictable floating point optimizations.
  float* test = malloc(sizeof(float) * w1 * w1);
  convolve_kernel_square_2d_slow(fin, test, w1, convolution, w2);
  for(unsigned i = 0; i < w1 * w1; i++) {
    assert(fabs(test[i] - fout[i]) < EPSILON);
  }
  free(test);
  #endif
}

void convolve_kernel_square_inplace_2d(float* f, unsigned w1, float* k, unsigned w2) {
  float ocpy[w1 * w1];
  memcpy(ocpy, f, w1 * w1 * sizeof(float));
  convolve_kernel_square_2d(ocpy, f, w1, k, w2);
}

void convolve_kernel_blur_33(const float* fin, float* fout, unsigned w) {
  convolve_kernel_square_2d(fin, fout, w, blur_33_kernel, 1);
}

void convolve_kernel_blur_33_inplace(float* f, unsigned w) {
  convolve_kernel_square_inplace_2d(f, w, blur_33_kernel, 1);
}

void convolve_kernel_blur_2d(const float* fin, float* fout, unsigned w1, unsigned w2) {
  float kernel[(w2 + 1) * (w2 + 1)];
  round_blur_kernel_2d(kernel, w2);
  convolve_kernel_square_2d(fin, fout, w1, kernel, w2);
}

void convolve_kernel_blur_inplace(float* f, unsigned w1, unsigned w2) {
  float kernel[(w2 * 2 + 1) * (w2 * 2 + 1)];
  round_blur_kernel_2d(kernel, w2);
  convolve_kernel_square_inplace_2d(f, w1, kernel, w2);
}

//TODO could be made more efficient if quadrant repetition were reused.
void round_blur_kernel_2d(float* out, int w) {
  unsigned tw = w * 2 + 1;
  //We use 1 / (1 + d^2 / w) for the unnormalized weight.
  float weight = 0;
  for(int y = -w; y <= w; y++) {
    for(int x = -w; x <= w; x++) {
      int yy = y + w;
      int xx = x + w;
      float dSqr = (x * x + y * y);
      float val = 1.0 / (1 + dSqr / w);
      out[yy * tw + xx] = val;
      weight += val;
    }
  }
  
  float scale = 1.0 / weight;
  for(unsigned i = 0; i < tw * tw; i++) {
    out[i] *= scale;
  }
  
  /*
  float scale = 1.0 / (4 * w);
  
  //Scale and copy to other quadrants.  Skips the x and y axes.
  for(int y = -w; y < 0; y++) {
    for(int x = -w; x < 0; x++) {
      int yy = y + w;
      int xx = x + w;
      
      //Indices of the corresponding value in each quadrant.
      unsigned iQ1 = yy * tw + (tw - xx - 1);
      unsigned iQ2 = yy * tw + xx;
      unsigned iQ3 = (tw - yy - 1) * tw + xx;
      unsigned iQ4 = (tw - yy - 1) * tw + (tw - xx - 1);
      out[iQ1] = out[iQ2] = out[iQ3] = out[iQ4] = out[iQ2] * scale;
    }
  }
  
  //Scale the 
  for(int i = -w; i <= w; i++) {
  
  } 
  
  */
}

float blur_33_kernel[9] = {
  1.0 / 16.0, 1.0 / 8.0, 1.0 / 16.0,
  1.0 /  8.0, 1.0 / 4.0, 1.0 / 8.0,
  1.0 / 16.0, 1.0 / 8.0, 1.0 / 16.0
};

