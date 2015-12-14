#ifndef CONVOLUTION_H
#define CONVOLUTION_H

//Calculates the matrix dot product between the submatrix of m1 (a matrix of width w1) centered at (x, y), with m2, a matrix of width "w2".  No bounds checking is performed, so use with caution.
float calculateMatrixDotNoBC(const float* m1, unsigned w1, unsigned x, unsigned y, const float* m2, int w2);
float calculateMatrixDot(const float* m1, unsigned w1, unsigned x, unsigned y, const float* m2, int w2);

//Apply a convolution kernel to f1, a matrix of width w1.  Kernel, of radius w2, givn by convolution.  Result stored in fout, a matrix of width w1.
void convolve_kernel_square_2d(const float* fin, float* fout, unsigned w1, const float* convolution, unsigned w2);

void convolve_kernel_blur_33(const float* fin, float* fout, unsigned w);
void convolve_kernel_blur_33_inplace(float* fin, unsigned w);
extern float blur_33_kernel[9];

#endif

