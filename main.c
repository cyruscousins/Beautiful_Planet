#include <stdio.h>
#include <stdlib.h>

#include "wind.h"
#include "image.h"
#include "draw.h"
#include "potentials.h"
#include "noise.h"
#include "parametric.h"

#define PI 3.14159627
#define TAU (2*PI)

//Create a tiny test ppm
void test1() {
  image* i = image_new(10, 10);
  fill_image(i, 0, 1, 0);
  fill_rect(i, 2, 2, 8, 8, 1, 0, 0.5);

  FILE* f = fopen("test.ppm", "wb");

  image_write_ppm(i, f, 255);
  //image_print(i, stdout);
}

//Create a large colored ppm
void test2() {
  image* i = image_new(1000, 1000);
  fill_image(i, 1, 1, 1);
  fill_rect(i, 100, 100, 900, 900, 0.5, 0.5, 0.5);
  fill_rect(i, 200, 200, 800, 800, 0, 0, 0);
  fill_rect(i, 300, 300, 700, 700, 1, 0, 0);
  fill_rect(i, 400, 400, 600, 600, 0, 1, 0);
  fill_rect(i, 450, 450, 550, 550, 0, 0, 1);

  FILE* f = fopen("test2.ppm", "wb");

  image_write_ppm(i, f, 255);
  //image_print(i, stdout);
}

//Test wind over time with the distance squared potential
void test3() {
  #define ISIZE 1000
  
  image* i = image_new(ISIZE, ISIZE);
  image* img = i;
  fill_image(i, 1, 1, 1);
  
  #define WSIZE 1000
  #define VMAX 4
  wind* w = wind_new(WSIZE);
  wind_randomize(w, ISIZE, 0, 0, ISIZE, ISIZE, VMAX, 100);
  
  centered_cl orbitcl = { ISIZE / 2.0, ISIZE / 2.0, 100 * 100, 1 };
  
  wind_draw(w, i, 0, 0, 0, 1, 0, 0, 1);
  
  #define STEPS 100
  for(unsigned i = 0; i < STEPS; i++) {
    wind_update(w, 0.1, distanceSquaredPotential, &orbitcl);
    wind_draw(w, img, (i % 100) / 100.0, (i % 50) / 50.0, (i % 25) / 25.0, 100.0 / STEPS, 0, 0, 1);
  }
  
  FILE* f = fopen("test3.ppm", "wb");
  image_write_ppm(i, f, 255);
  
  //wind_print(w, stdout);
}

//Test the noise function.
void test4() {

#undef ISIZE
#define ISIZE 256

  image* img = image_new(ISIZE, ISIZE);
  fill_image(img, 1, 1, 1);

  noise_sum* noiseFunctions[3];
  
  for(unsigned i = 0; i < 3; i++) {
    noiseFunctions[i] = initialize_noise_sum_2d(16, 4);
    noise_sum_scale_in(noiseFunctions[i], 2 * 1.0 / (ISIZE)); //Should cause 4 identical tiles.
  }
  
  for(unsigned y = 0; y < ISIZE; y++) {
    for(unsigned x = 0; x < ISIZE; x++) {
      float intensity = noise_sum_2d(x, y, noiseFunctions[0]);
      
      for(unsigned c = 0; c < C; c++) {
        *image_pixel(img, x, y, c) = intensity;
        //*image_pixel(img, x, y, c) = noise_sum_2d(xp, yp, noiseFunctions[c]) * 2;
        //printf("%f ", noise_sum_2d(xp, yp, noiseFunctions[c]));
      }
    }
  }
  
  FILE* f = fopen("test4.ppm", "wb");
  image_write_ppm(img, f, 255);
  //image_print(img, stdout);
}

//Test drawing many winds
void test5() {
  #undef ISIZE
  #define ISIZE 2000
  //#define ISIZE 100

  image* img = image_new(ISIZE, ISIZE);
  fill_image(img, 0, 0, 0);
  
  noise_sum* ncl = initialize_noise_sum_2d(4, 6);
  noise_sum_scale_in(ncl, 1.0 / ISIZE);

  //fill_rect_f_bw(img, 0, 0, ISIZE, ISIZE, noisePaint, ncl);
  
  #undef WSIZE
  #define WSIZE 50
  
  #undef STEPS
  #define STEPS 5000
  
  #define WIND_COLORS 7
  wind* winds[WIND_COLORS];
  for(unsigned i = 0; i < WIND_COLORS; i++) {
    winds[i] = wind_new(WSIZE);
    wind_randomize(winds[i], ISIZE, 0, 0, ISIZE, ISIZE, VMAX, 100);
  }
  
  float windColors[WIND_COLORS * 3] = {
    0, 0, 1,
    0, 1, 0,
    0, 1, 1,
    1, 0, 0,
    1, 0, 1,
    1, 1, 0,
    1, 1, 1,
  };

  centered_cl orbitcl = { ISIZE / 2.0, ISIZE / 2.0, 1000 * 1000, 100 };
  
  #define POTENTIAL_COUNT 2
  float(*potentialFunctions[POTENTIAL_COUNT])(float, float, void*) = {
    noiseSumPotential,
    distanceSquaredPotential
  };
  
  void* cl[POTENTIAL_COUNT] = {ncl, &orbitcl};
  
  float weights[POTENTIAL_COUNT] = {
    100, 10
  };
  
  poly_weighted_cl pcl = { potentialFunctions, cl, weights, POTENTIAL_COUNT};
  
  for(unsigned i = 0; i < STEPS; i++) {
    float a = (1 + i) / (float)STEPS;
    unsigned particles = 3;
    unsigned spread = 1;
    particles += (STEPS - i) * 3 / (STEPS);
    spread += (STEPS - i) * 8 / (STEPS);
    
    for(unsigned j = 0; j < WIND_COLORS; j++) {
      wind_update(winds[j], 1.0, sumWeightedPotential, &pcl);
      //wind_update(winds[j], 1.0, noiseSumPotential, ncl);
      wind_draw(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a, 0, 0, 1);
      wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a * 0.75, 0, 0, 1, particles, spread);
    }
  }
  
  FILE* f = fopen("test5.ppm", "wb");
  image_write_ppm(img, f, 255);
}

//Test drawing parametric curves, drawing circles.
void test6() {
  #undef ISIZE
  #define ISIZE 3000
  image* img = image_new(ISIZE, ISIZE);
  fill_image(img, 1, 1, 1);
  
  color drawcl = {0, 0, 0, 1};
  circle_cl drawcl_2 = {1, 0, 1, 0.5, 5.5};
  
  #define REPLICANTS 4
  for(unsigned x = 0; x < REPLICANTS; x++) {
    //Draw the first parametric curve type here.
    
    float x0 = (x + 0.5) * ISIZE / REPLICANTS;
    float y0 = ISIZE / 3.0;
    float scale = ISIZE * 0.5 / REPLICANTS;
    
    ccl_1 cl;
    randomize_ccl_1(&cl, x0, y0, scale);
    
    draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_1, 0, TAU, 0.01 / scale, &drawcl, &cl);
    draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_1, 0, TAU, 0.01, 8, 0.9, &drawcl_2, &cl);
    
    //Draw the second parametric curve type here.
    
    y0 = ISIZE * 2 / 3;
    
    ccl_2 cl2;
    randomize_ccl_2(&cl2, x0, y0, scale);
    
    /*
    printf("%f %f %f %f", cl2.x0, cl2.y0, cl2.xS, cl2.yS);
    for(unsigned i = 0; i < 4; i++) {
      printf(" %f %f %f", cl2.d[i], cl2.f[i], cl2.p[i]);
    }
    printf("\n");
    */
    
    //draw_parametric_curve_uniform_time(img, draw_circle, parametric_curve_2, 0, TAU, 0.01 / scale, &drawcl_2, &cl2);
    draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_2, 0, 100, 0.01, &drawcl, &cl2);
    draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_2, 0, 100, 0.01, 8, 0.9, &drawcl_2, &cl2);
  }
  
  FILE* f = fopen("test6.ppm", "wb");
  image_write_ppm(img, f, 255);
}

//Test moving wind sources along parametric curves.
void test7() {
  image* img = image_new(ISIZE, ISIZE);
  fill_image(img, 0, 0, 0);
  
  #undef WSIZE
  #define WSIZE 4000
  
  #define WIND_CURVES 8
  
  wind* winds[WIND_CURVES];
  for(unsigned i = 0; i < WIND_CURVES; i++) {
    winds[i] = wind_new(WSIZE);
    //wind_randomize(winds[i], ISIZE, 0, 0, ISIZE, ISIZE, VMAX, 100);
  }
  
  float windColors[WIND_CURVES * 3] = {
    0, 0, 1,
    0, 1, 0,
    1, 0, 0,
    0, 1, 1,
    1, 0, 1,
    1, 1, 0,
    1, 1, 1,
    0.5, 0.5, 0.5,
  };
  
  ccl_1 pc1_cl[WIND_CURVES / 2];
  ccl_2 pc2_cl[WIND_CURVES / 2];
  
  //Parametric curve functions
  vec2(*parametricCurves[WIND_CURVES])(float, void*);  
  
  //Closures for parametric curve functions
  void* cl[WIND_CURVES];
  
  for(unsigned i = 0; i < WIND_CURVES / 2; i++) {
    randomize_ccl_1(pc1_cl + i, ISIZE / 2, ISIZE / 2, ISIZE / 2);
    randomize_ccl_2(pc2_cl + i, ISIZE / 2, ISIZE / 2, ISIZE / 2 * 8);
    
    cl[i] = pc1_cl + i;
    parametricCurves[i] = parametric_curve_1;
    
    cl[i + WIND_CURVES / 2] = (void*) pc2_cl + i;
    parametricCurves[i + WIND_CURVES / 2] = parametric_curve_2;
  }
  
  
  //Potential Function:
  noise_sum* ncl = initialize_noise_sum_2d(4, 4);
  noise_sum_scale_in(ncl, 1.0 / ISIZE);

  centered_cl orbitcl = { ISIZE / 2.0, ISIZE / 2.0, 1000 * 1000, 100 };
  
  float(*potentialFunctions[POTENTIAL_COUNT])(float, float, void*) = {
    noiseSumPotential,
    distanceSquaredPotential
  };
  
  void* pcls[POTENTIAL_COUNT] = {ncl, &orbitcl};
  
  float weights[POTENTIAL_COUNT] = {
    100, 0.1
  };
  
  poly_weighted_cl pcl = { potentialFunctions, pcls, weights, POTENTIAL_COUNT};
  
  
  //Move along the curves, adding to the wind.
  
  float aMax = 0.125;
  
  float tStep = TAU / WSIZE;
  for(unsigned i = 0; i < WSIZE; i++) {
    float t = tStep * i;
    float a = (1 + (1.0 + i) / (WSIZE) * aMax) / 2;
    for(unsigned j = 0; j < 3; j++) { //Just draw 3 curves
      vec2 v = parametricCurves[j](t, cl[j]);
      
      wind_append(winds[j], v.x, v.y, uniformFloat(-0.125, 0.125), uniformFloat(-0.125, 0.125), uniformFloat(1, 2));
      
      if(i % 4 == 0) {
        wind_update_bound(winds[j], 0.5, sumWeightedPotential, &pcl, -ISIZE / 2.0, -ISIZE / 2.0, 3.0 * ISIZE / 2, 3.0 * ISIZE / 2);

        //Draw the wind.
        //wind_draw(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a, 0, 0, 1);
        wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a * 0.5, 0, 0, 1, 5, 5);
      }
    }
  }
  
  
  //Draw the wind
  /*
  for(unsigned j = 0; j < WIND_CURVES; j++) {
    wind_draw(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], aMax, 0, 0, 1);
    wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], aMax * 0.5, 0, 0, 1, 5, 5);
  }
  */
  
  
  //Render
  FILE* f = fopen("test7.ppm", "wb");
  image_write_ppm(img, f, 255);
}

int main(int argc, char** argsv) {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
}

