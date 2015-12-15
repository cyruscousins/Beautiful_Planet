#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "wind.h"
#include "image.h"
#include "draw.h"
#include "potentials.h"
#include "noise.h"
#include "parametric.h"
#include "filters.h"

#define PI 3.14159627
#define TAU (2*PI)

//TODO
//Some globals respected by (most of) the tests.
int imageWidth = 1000;
unsigned frames = 100;
char* testPrefix;

unsigned nsSize = 16;
unsigned nsDepth = 4;

char fileBuffer[256];

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
  image* i = image_new(imageWidth, imageWidth);
  image* img = i;
  fill_image(i, 1, 1, 1);
  
  #define WSIZE 1000
  #define VMAX 4
  wind* w = wind_new(WSIZE);
  wind_randomize(w, imageWidth, 0, 0, imageWidth, imageWidth, VMAX, 100);
  
  centered_cl orbitcl = { imageWidth / 2.0, imageWidth / 2.0, 100 * 100, 1 };
  
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
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 1, 1, 1);

  noise_sum* noiseFunctions[3];
  
  for(unsigned i = 0; i < 3; i++) {
    noiseFunctions[i] = initialize_noise_sum_2d(nsSize, nsDepth);
    noise_sum_scale_in(noiseFunctions[i], 2 * 1.0 / (imageWidth)); //Should cause 4 identical tiles.
  }
  
  for(unsigned y = 0; y < imageWidth; y++) {
    for(unsigned x = 0; x < imageWidth; x++) {
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
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 0, 0, 0);
  
  noise_sum* ncl = initialize_noise_sum_2d(4, 6);
  noise_sum_scale_in(ncl, 1.0 / imageWidth);

  //fill_rect_f_bw(img, 0, 0, imageWidth, imageWidth, noisePaint, ncl);
  
  #undef WSIZE
  #define WSIZE 50
  
  #undef STEPS
  #define STEPS 5000
  
  #define WIND_COLORS 7
  wind* winds[WIND_COLORS];
  for(unsigned i = 0; i < WIND_COLORS; i++) {
    winds[i] = wind_new(WSIZE);
    wind_randomize(winds[i], imageWidth, 0, 0, imageWidth, imageWidth, VMAX, 100);
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

  centered_cl orbitcl = { imageWidth / 2.0, imageWidth / 2.0, 1000 * 1000, 100 };
  
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
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 1, 1, 1);
  
  color drawcl = {0, 0, 0, 1};
  circle_cl drawcl_2 = {1, 0, 1, 0.5, 5.5};
  
  #define REPLICANTS 8
  #define ROWS 5
  for(unsigned x = 0; x < REPLICANTS; x++) {
    float x0 = (x + 0.5) * imageWidth / REPLICANTS;
    float y0 = imageWidth * 0.5 / ROWS;
    float scale = imageWidth * 0.5 / REPLICANTS;
    
    float spacing = 10.0 / imageWidth;
    
    
    //Draw the first parametric curve type here.
    
    ccl_1 cl;
    randomize_ccl_1(&cl, x0, y0, scale);
    
    draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_1, 0, TAU, 0.01 / scale, &drawcl, &cl);
    draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_1, 0, TAU, 0.01, 8, 0.9, &drawcl_2, &cl);
    
    //Draw the second parametric curve type here.
    
    y0 = imageWidth * 1.5 / ROWS;
    
    ccl_2 cl2;
    randomize_ccl_2(&cl2, x0, y0, scale);
    
    //draw_parametric_curve_uniform_time(img, draw_circle, parametric_curve_2, 0, TAU, 0.01 / scale, &drawcl_2, &cl2);
    draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_2, 0, 100, spacing, &drawcl, &cl2);
    draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_2, 0, 100, spacing, 8, 0.9, &drawcl_2, &cl2);
    
    //Draw the third parametric curve type here.
    
    y0 = imageWidth * 2.5 / ROWS;
    
    cycloid cl3;
    randomize_hypocycloid(&cl3, x0, y0, scale);
    draw_parametric_curve_uniform_time(img, draw_point, hypocycloid, 0, 100, spacing, &drawcl, &cl3);
    draw_parametric_curve_uniform_space(img, draw_circle, hypocycloid, 0, 100, spacing, 8, 0.9, &drawcl_2, &cl3);
    
    //Draw the fourth parametric curve type here.
    y0 = imageWidth * 3.5 / ROWS;
    
    cycloid cl4;
    randomize_epicycloid(&cl4, x0, y0, scale);
    draw_parametric_curve_uniform_time(img, draw_point, epicycloid, 0, 100, spacing, &drawcl, &cl4);
    draw_parametric_curve_uniform_space(img, draw_circle, epicycloid, 0, 100, spacing, 8, 0.9, &drawcl_2, &cl4);
    
    
    //Draw the fifth (sum) parametric curve type here.
    y0 = imageWidth * 4.5 / ROWS;
    
    weighted_sum_pcl* cl5 = randomize_weighted_sum(x0, y0, scale, 8);
    draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_weighted_sum, 0, 100, spacing * 0.1, &drawcl, cl5);
    //draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_weighted_sum, 0, 100, spacing, 8, 0.9, &drawcl_2, cl5);
    
    free_weighted_sum(cl5);
  }
  
  FILE* f = fopen("test6.ppm", "wb");
  image_write_ppm(img, f, 255);
}

//Test moving wind sources along parametric curves.
void test7() {
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 0, 0, 0);
  
  #undef WSIZE
  #define WSIZE 1000
  
  #define WIND_CURVES 8
  
  wind* winds[WIND_CURVES];
  for(unsigned i = 0; i < WIND_CURVES; i++) {
    winds[i] = wind_new(WSIZE);
    //wind_randomize(winds[i], imageWidth, 0, 0, imageWidth, imageWidth, VMAX, 100);
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
    randomize_ccl_1(pc1_cl + i, imageWidth / 2, imageWidth / 2, imageWidth / 4);
    randomize_ccl_2(pc2_cl + i, imageWidth / 2, imageWidth / 2, imageWidth / 2 * 8);
    
    cl[i] = pc1_cl + i;
    parametricCurves[i] = parametric_curve_1;
    
    cl[i + WIND_CURVES / 2] = (void*) pc2_cl + i;
    parametricCurves[i + WIND_CURVES / 2] = parametric_curve_2;
  }
  
  
  //Potential Function:
  noise_sum* ncl = initialize_noise_sum_2d(nsSize, nsDepth);
  noise_sum_scale_in(ncl, 1.0 / imageWidth);

  centered_cl orbitcl = { imageWidth / 2.0, imageWidth / 2.0, 1000 * 1000, 100 };
  
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
  
  float aMax = 1.0 / 32.0;
  
  float tStep = TAU / WSIZE;
  for(unsigned i = 0; i < WSIZE; i++) {
    float t = tStep * i;
    float a = (1 + (1.0 + i) / (WSIZE) * aMax) / 2;
    for(unsigned j = 0; j < 3; j++) { //Just draw 3 curves
      vec2 v = parametricCurves[j](t, cl[j]);
      
      wind_append(winds[j], v.x, v.y, uniformFloat(-0.125, 0.125), uniformFloat(-0.125, 0.125), uniformFloat(1, 2));
      
      //wind_update_bound(winds[j], 0.5, sumWeightedPotential, &pcl, -imageWidth / 2.0, -imageWidth / 2.0, 3.0 * imageWidth / 2, 3.0 * imageWidth / 2);
      wind_update(winds[j], 0.25, sumWeightedPotential, &pcl);

      //Draw the wind.
      //wind_draw(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a, 0, 0, 1);
      wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], a * 0.5, 0, 0, 1, 3, 3);
    }
  }
  
  
  //Draw the wind
  /*
  for(unsigned j = 0; j < WIND_CURVES; j++) {
    wind_draw(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], aMax, 0, 0, 1);
    wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], aMax * 0.5, 0, 0, 1, 5, 5);
  }
  */
  
  //Apply some blur.
  image* img2 = image_blur(img, 2);
  
  //Render
  FILE* f = fopen("test7.ppm", "wb");
  image_write_ppm(img, f, 255);
  
  fclose(f);
  f = fopen("test7-blur.ppm", "wb");
  image_write_ppm(img2, f, 255);
  fclose(f);
  
  image_free(img);
  image_free(img2);
  for(unsigned i = 0; i < WIND_CURVES; i++) {
    wind_free(winds[i]);
  }
  noise_sum_free(ncl);
}

//Essentially an animated version of test 7.
void test8() {
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 0, 0, 0);
  
  //Parameterization:
  unsigned wind_count = 8; //Number of particle emmiters.
  unsigned wind_size = 1000; //Max number of particles per emitter.
  unsigned particlesPerFrame = 2000 / frames;
  if(particlesPerFrame == 0) particlesPerFrame = 1;
  
  unsigned particlesToDraw = 2;
  float maxSpread = imageWidth / 500.0;
  
  float particleNoiseVelocity = imageWidth / (32.0 * 1000.0);
  float particleEmitterMomentumScalar = 0.25;
  
  float totalTime = TAU;
  
  float wColorRevert = 0.01; //TODO these should be exponentiated by tStep
  float wColorChange = 0.125;
  
  float wAlpha = 1.0 / 8.0; //Alpha to use for drawing the wind.
  
  unsigned tracer_count = 8; //Number of traced curves.
  float tracerColorChange = 0.125;
  float tracerColorReduction = 0.9;
  
  ////////////////////
  //Particle Emitters:
  
  wind* winds[wind_count];
  for(unsigned i = 0; i < wind_count; i++) {
    winds[i] = wind_new(WSIZE);
    //wind_randomize(winds[i], imageWidth, 0, 0, imageWidth, imageWidth, VMAX, 100);
  }
  
  float windDefaultColors[wind_count * 3];
  float windColors[wind_count * 3];
  
  for(unsigned i = 0; i < wind_count; i++) {
    windDefaultColors[i * 3 + 0] = (i & 0x1) ? 3 : 0;
    windDefaultColors[i * 3 + 1] = (i & 0x2) ? 3 : 0;
    windDefaultColors[i * 3 + 2] = (i & 0x4) ? 3 : 0;
  };
  memcpy(windColors, windDefaultColors, sizeof(float) * wind_count * 3);
  
  //////////////
  //Wind Curves:
  
  //Wind curve closures (Specific type storage).
  ccl_1 pc1_cl[wind_count / 2];
  ccl_2 pc2_cl[wind_count / 2];
  
  //Parametric curve functions (Indirection layer to support generic curve types).
  vec2(*parametricCurves[wind_count])(float, void*);  
  
  //Closures for (generic) parametric curve functions
  void* cl[wind_count];
    
  //Initialize wind closures.
  for(unsigned i = 0; i < wind_count / 2; i++) {
    randomize_ccl_1(pc1_cl + i, imageWidth / 2, imageWidth / 2, imageWidth / 4);
    //Keep the amplitude low and the base high.
    pc1_cl[i].base = (pc1_cl[i].base + imageWidth / 2) / 2;
    pc1_cl[i].amplitude *= 0.5;
    pc1_cl[i].frequency = 2 + pc1_cl[i].frequency / 4;
    
    cl[i] = (void*) (pc1_cl + i);
    parametricCurves[i] = parametric_curve_1;
    
    randomize_ccl_2(pc2_cl + i, imageWidth / 2, imageWidth / 2, imageWidth / 2);
    for(unsigned j = 0; j < 4; j++) {
      pc2_cl[i].d[j] *= 1.0 / 32.0;
    }
    
    cl[i + wind_count / 2] = (void*) (pc2_cl + i);
    parametricCurves[i + wind_count / 2] = parametric_curve_2;
  }
  
  /////////////////////
  //Potential Function:
  
  noise_sum* ncl = initialize_noise_sum_2d(nsSize, nsDepth);
  noise_sum_scale_in(ncl, 1.0 / imageWidth);

  centered_cl orbitcl = { imageWidth / 2.0, imageWidth / 2.0, imageWidth * imageWidth, (imageWidth / 16.0) * (imageWidth / 16.0) };
  
  float(*potentialFunctions[POTENTIAL_COUNT])(float, float, void*) = {
    noiseSumPotential,
    distanceSquaredPotential
  };
  
  void* pcls[POTENTIAL_COUNT] = {ncl, &orbitcl};
  
  float weights[POTENTIAL_COUNT] = {
    25, 1.25
  };
  
  poly_weighted_cl pcl = { potentialFunctions, pcls, weights, POTENTIAL_COUNT};
  
  //////////
  //Tracers:
  
  //Offsets in time
  float timeOffsets[tracer_count];
  ccl_2 tracer_cl[tracer_count];
  color tracer_color[tracer_count];
  
  memset(tracer_cl, 0, tracer_count * sizeof(ccl_2));
  memset(tracer_color, 0, tracer_count * sizeof(color));
  
  for(unsigned i = 0; i < tracer_count; i++) {
    tracer_cl[i].x0 = tracer_cl[i].y0 = -128;
    tracer_cl[i].scale = 0;
    timeOffsets[i] = 0;
    
    //Start off offscreeen.
    
    //randomize_ccl_2(tracer_cl + i, imageWidth / 2, imageWidth / 2, imageWidth / 2);
    //randomize_color(tracer_color + i, -1, 2, 1);
  }
  
  /////////////////////
  //Run the Simulation:
      
  //Move along the curves, adding to the wind.
  
  float tStep = totalTime / frames;
  float drag = powf(0.5, tStep);
  
  unsigned particleCount = 0; //Keep track of the total number of particles.
  for(unsigned i = 0; i < frames; i++) {
    float t = tStep * i;
    //if(i % 10 == 0) {
      printf("Frame %u / %u: Particles %u / %u\r", i, frames, particleCount, WSIZE * wind_count);
      fflush(stdout);
    //}
    particleCount = 0;
    
    //Blur and darken the existing image.
    /*
    fill_image_a(img, 0, 0, 0, 1.0 / 32);
    image_blur_inplace(img, (i / 4) % 1);
    */
    
    if(i % 2 == 0) {
      float m = 1.0 - 64.0 / imageWidth;
      if(m < 0) m = 0;
      //Darken one color channel at a time.
      image_multiply_channel(img, (i / 2) % 3, m);
      //fill_image_a(img, 0, 0, 0, 32.0 / imageWidth);
    } else {
      image_blur_fast_inplace(img, i / 2);
    }
    
    //"Gravity Vortex": bend light inward.  (TODO move this to filters).
    /*
    for(unsigned i = 0; i < imageWidth * imageWidth / 128; i++) {
      vec2 p = symmetricBall(imageWidth / 4);
      unsigned x0 = imageWidth / 2 + (unsigned)p.x;
      unsigned y0 = imageWidth / 2 + (unsigned)p.y;
      unsigned x1 = imageWidth / 2 + (unsigned)(p.x * 1.125);
      unsigned y1 = imageWidth / 2 + (unsigned)(p.y * 1.125);
      for(unsigned c = 0; c < C; c++) {
        float* p0 = image_pixel(img, x0, y0, c);
        float* p1 = image_pixel(img, x1, y1, c);
        *p0 = (*p0 + *p1 * 0.75);
        *p1 *= 0.25;
      }
    }
    */
    
    //image_blur_fast_inplace(img, i);
    //image_blur_fast_inplace(img, i + 1);
    
    if(i % 32 == 0) {
      //Reset a curve
      unsigned idx = uniformInt(0, tracer_count);
      
      randomize_ccl_2(tracer_cl + idx, imageWidth / 2, imageWidth / 2, imageWidth / 2);
      
      for(unsigned j = 0; j < 4; j++) {
        tracer_cl[idx].d[j] *= 1.0 / 32.0;
      }
      
      
      //Randomize color.
      //randomize_color(tracer_color + idx, -0.5, 1.5, 1);
      //tracer_color[idx].a = uniformFloat(-.25, 1.5); //Negative alpha can cause some interesting effects (existing color is emphasized, new is subtracted).
      
      //Randomize color, chosen to work well with additive coloring.
      randomize_color(tracer_color + idx, -0.125, 0.5, 1);
      timeOffsets[idx] = t;
    }
    
    /*
    if(i % 64 == 0) {
      randomize_ccl_2(&ccl, imageWidth / 2, imageWidth / 2, imageWidth);
      
      for(unsigned i = 0; i < 4; i++) {
        ccl.d[i] *= 1.0 / 16.0;
      }
      
      //draw_parametric_curve_uniform_space(img, draw_circle, parametric_curve_1, 0, TAU, 0.01, 8, 0.9, &drawcl_2, &cl);
    }
    */
    
    /*
    float c2Speed = 1;
    {
      //color drawcl = {uniformFloat(0, 4), uniformFloat(0, 4), uniformFloat(0, 4), uniformFloat(1.0 / 16.0, 1)};
      
      color drawcl = {uniformFloat(-1, 2), uniformFloat(-1, 2), uniformFloat(-1, 2), uniformFloat(1.0 / 16.0, 1)};
      
      circle_cl drawcl_2 = {1, 0, 1, 0.5, 5.5};
      
      draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_2, i % 64 * c2Speed, (i % 64 + 1) * c2Speed, 0.01, &drawcl, &ccl);
      
      //ccl.xS *= 0.95;
      //ccl.yS *= 0.95;
    }
    */
    
    //Render a partial curve.
    
    float tSpeed = 16.0;
    for(unsigned j = 0; j < tracer_count; j ++) {
      float renderSpacing = (1.0 / tracer_cl[j].scale) * (1.0);
      
      if(tracer_cl[j].scale < 1) continue;
      
      //Draw a partial curve
      if(j < tracer_count / 2) {
      
        float tt = (t - timeOffsets[j]) * tSpeed;
        float tt2 = (t - timeOffsets[j] + tStep) * tSpeed;
        
        //draw_parametric_curve_uniform_time(img, draw_point, parametric_curve_2, 0, uniformFloat(0, 200), 0.01, &drawcl, cl[j]);
        
        draw_parametric_curve_uniform_time(img, draw_point_additive, parametric_curve_2, tt, tt2, renderSpacing * 0.1, &tracer_color[j], &tracer_cl[j]);
        //color drawcl2 = {windColors[j * 3 + 1], windColors[j * 3 + 2], windColors[j * 3 + 0], 3};
        //draw_parametric_curve_uniform_time(img, draw_point, parametricCurves[j], 0, tt, 0.01, &drawcl2, cl[j]);
      } else {
        //Flash and permute a full curve.
        
        if(uniformInt(0, 2) == 0) {
          continue;
        }
        
        float sd = 32.0;
        float pd = TAU / 128.0;
        
        tracer_cl[j].scale *= uniformFloat((sd - 1) / sd, (sd + 1) / sd); //Should shrink slightly over time.
        tracer_cl[j].theta += uniformFloat(-pd, pd);
        
        
        float dpMax = imageWidth / 1000.0;
        vec2 dp = vScale(dpMax, symmetricUnitBall());
        tracer_cl[j].x0 += dp.x;
        tracer_cl[j].y0 += dp.y;
        
        
        float dd = 128.0;
        for(unsigned i = 0; i < PC2_SUMMANDS; i++) {
          tracer_cl[j].d[i] *= uniformFloat((dd - 1) / dd, (dd + 1) / dd);
          tracer_cl[j].f[i] *= uniformFloat((dd - 1) / dd, (dd + 1) / dd);
          tracer_cl[j].p[i] += uniformFloat(-pd, pd);
        }
        
        draw_parametric_curve_uniform_time(img, draw_point_additive, parametric_curve_2, 0, uniformFloat(0, 200), renderSpacing * (1 + uniformFloat(0, 1)), &tracer_color[j], &tracer_cl[j]);
      }
    }
    
    //Permute tracer colors.
    for(unsigned i = 0; i < tracer_count; i++) {
      for(unsigned c = 0; c < C; c++) {
        tracer_color[i].c[c] += uniformFloat(-tracerColorChange, tracerColorChange);
        tracer_color[i].c[c] *= uniformFloat(tracerColorReduction, 1);
      }
      //tracer_color[i].a += uniformFloat(-tracerColorChange, tracerColorChange);
      //tracer_color[i].a *= tracerColorReduction;
    }
    
    for(unsigned i = 0; i < wind_count * 3; i++) {
      windColors[i] = windColors[i] * (1 - wColorRevert) + windDefaultColors[i] * wColorRevert + uniformFloat(-wColorChange, wColorChange);
    }
    
    for(unsigned j = 0; j < wind_count; j++) {
      //Randomly remove some particles.
      wind_remove_rand(winds[j], 512);
      //Apply a bit of drag.
      wind_scale_velocity(winds[j], drag);
      
      //Calculate the position and velocity of the emitter.
      vec2 v = parametricCurves[j](t, cl[j]);
      vec2 v2 = parametricCurves[j](t + EPSILON, cl[j]);
      vec2 dcdt = vScale((tStep / EPSILON), vMinus(v2, v));
      
      for(unsigned k = 0; k < particlesPerFrame; k++) {
        vec2 noise = vScale((tStep * particleNoiseVelocity), symmetricUnitBall());
        vec2 dvdt = vPlus(vScale(particleEmitterMomentumScalar, dcdt), noise);
        particleNoiseVelocity = imageWidth / (32.0 * 1000.0);
        
        wind_append(winds[j], v.x, v.y, dvdt.x, dvdt.y, uniformFloat(1, 2));
      }
      
      //Update each particle.
      wind_update_bound(winds[j], 1.0, sumWeightedPotential, &pcl, -imageWidth / 2.0, -imageWidth / 2.0, 3.0 * imageWidth / 2, 3.0 * imageWidth / 2);
      //wind_update(winds[j], 0.5, sumWeightedPotential, &pcl);

      //Draw the wind.
      wind_draw_roffset(winds[j], img, windColors[j * 3 + 0], windColors[j * 3 + 1], windColors[j * 3 + 2], wAlpha, 0, 0, 1, particlesToDraw, maxSpread);
      
      particleCount += winds[j]->particles;
      
      //TODO want to change the noise over time so wind does not collect in low pressure areas.
      
      //TODO render the noise function.
      
      //TODO periodically reset wind emmiters for type 2 curves.
    }
    
    //Render
    char path[128];
    sprintf(path, "test_video_8/%05u.ppm", i);
    FILE* f = fopen(path, "wb");
    image_write_ppm(img, f, 255);
    fclose(f);
  }
  
  image_free(img);
  for(unsigned i = 0; i < wind_count; i++) {
    wind_free(winds[i]);
  }
  noise_sum_free(ncl);
}

//Curve perturbation animation test.
void test9() {
  image* img = image_new(imageWidth, imageWidth);
  fill_image(img, 0, 0, 0);
  
  //Parameterization:
  
  float totalTime = 400;
    
  unsigned tracer_count = 8; //Number of traced curves.
  float tracerColorChange = 1.0 / 16.0;
  float tracerColorReduction = 0.99;
  
  //////////
  //Tracers:
  
  weighted_sum_pcl_static* curve = randomize_weighted_static_sum(imageWidth / 2, imageWidth / 2, imageWidth / 2, 16);
  color color;
  randomize_color(&color, -0.125, 0.5, 1);
  
  /////////////////////
  //Run the Simulation:
      
  //Move along the curves, adding to the wind.
  
  float tStep = totalTime / frames;
  
  for(unsigned i = 0; i < frames; i++) {
    float t = tStep * i;
    //if(i % 10 == 0) {
      printf("Frame %u / %u\r", i, frames);
      fflush(stdout);
    //}
    
    //Blur and darken the existing image.
    
    if(i % 2 == 0) {
      float m = 1.0 - 512.0 / imageWidth;
      if(m < 0) m = 0;
      //Darken one color channel at a time.
      image_multiply_channel(img, (i / 2) % 3, m);
      //fill_image_a(img, 0, 0, 0, 32.0 / imageWidth);
    } else {
      image_blur_fast_inplace(img, i / 2);
    }
    
    //Render the curve
    float renderSpacing = (1.0 / imageWidth) * (10.0);
    
    //draw_parametric_curve_uniform_time(img, draw_point_additive, parametric_curve_weighted_static_sum, 0, uniformFloat(0, 200), renderSpacing * (1 + uniformFloat(0, 1)), &color, curve);
    draw_parametric_curve_uniform_space(img, draw_point_additive, parametric_curve_weighted_static_sum, 0, t, 0.01, 1.5, 0.9, &color, curve);
    
    //Perturb the curve  
    perturb_weighted_static_sum(curve, 0.005);
    
    //Perturb the colors.
    for(unsigned c = 0; c < C; c++) {
      color.c[c] += uniformFloat(-tracerColorChange, tracerColorChange);
      color.c[c] *= uniformFloat(tracerColorReduction, 1);
    }
    
    //Write to disk
    char path[128];
    sprintf(path, "test_video_9/%05u.ppm", i);
    FILE* f = fopen(path, "wb");
    image_write_ppm(img, f, 255);
    fclose(f);
  }
  
  image_free(img);
}

#define TESTCOUNT 9
void (*testFunctions[TESTCOUNT])() = {
  test1, test2, test3, test4, test5, test6, test7, test8, test9
};

#define bool char
#define TRUE 1
#define FALSE 0

int main(int argc, char** argsv) {
  bool runTest[TESTCOUNT];
  
  for(unsigned i = 0; i < TESTCOUNT; i++) {
    runTest[i] = FALSE;
  }
  
  for(unsigned i = 1; i < argc; i++) {
    int val;
    if(sscanf(argsv[i], "w=%d", &val)) {
      imageWidth = val;
      printf("Setting width of test images to %u.\n", imageWidth);
    } else if(sscanf(argsv[i], "s=%d", &val)) {
      nsSize = val;
      printf("Setting noise width to %u.\n", nsSize);
    } else if(sscanf(argsv[i], "d=%d", &val)) {
      nsDepth = val;
      printf("Setting noise depth to %u.\n", nsDepth);
    } else if(sscanf(argsv[i], "f=%d", &val)) {
      frames = val;
      printf("Setting frames to %u.\n", frames);
    } else if(sscanf(argsv[i], "r=%d", &val)) {
      seed_lcg((uint32_t)val);
      printf("Seeding random number generator with %u.\n", val);
    } else if(sscanf(argsv[i], "%d", &val)) {
      if(val == -1) {
        for(unsigned i = 0; i < TESTCOUNT; i++) {
          runTest[i] = TRUE;
        }
      } else if(val <= 0 || val > TESTCOUNT) {
        fprintf(stderr, "Can't run test %d: max is %d.  Terminating.\n", val, TESTCOUNT);
        return 1;
      } else {
        runTest[val - 1] = TRUE;
      }
    } else {
      fprintf(stderr, "Error: unrecognized option \"%s\".  Terminating.\n", argsv[i]);
    }
  }
  
  for(unsigned i = 0; i < TESTCOUNT; i++) {
    if(runTest[i]) {
      printf("Running test %u.\n", i + 1);
      testFunctions[i]();
    }
  }
}

