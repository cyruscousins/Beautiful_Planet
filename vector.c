#include "vector.h"

void draw_line(image* img, unsigned x, unsigned y, void *lp) {
    
    line_t* line_param = (line_t*) lp;
    
    color *cl = &line_param->c;

    vec2 dir_vec = vMinus(line_param->end, line_param->start);
    vec2 unit_dir_vec = vNormalize(dir_vec);

    vec2 start = line_param->start;
   
    double d = vDistance(line_param->end, line_param->start);

    for(int i = 0; i < d; i++) {
	draw_point(img, (unsigned)start.x + x, (unsigned)start.y + y, cl);
	start = vPlus(start, unit_dir_vec);
    }
}

void draw_chain(image* img, unsigned x, unsigned y, void *cp) {

    chain_t* chain_param = (chain_t*) cp;
    
    for (int i = 0; i < chain_param->num_points - 1; i++) {
	line_t lp;
        lp.start = chain_param->points[i];
	lp.end  = chain_param->points[i + 1];
	lp.c = chain_param->c;
	draw_line(img, x, y, &lp);
    }      
}

void draw_cchain(image* img, unsigned x, unsigned y, void *ccp) {
    
    cchain_t* cchain_param = (cchain_t*) ccp;
    for (int i = 0; i < cchain_param->num_chains; i++) {
        chain_t cp;
	cp.points = cchain_param->points[i];
	cp.num_points = cchain_param->num_points[i];
	cp.c = cchain_param->c[i];
	draw_chain(img, x, y, &cp);
    }
}
