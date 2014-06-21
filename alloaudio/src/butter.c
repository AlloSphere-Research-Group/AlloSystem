
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#include "alloaudio/butter.h"

struct BUTTER_ {
    double a0, a1, a2, b1, b2;
    double x_1, x_2, y_1, y_2;
    int lp;
    double const0;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846	/* pi */
#endif

BUTTER *butter_create(int sr, int lp)
{
    BUTTER *b = (BUTTER *)malloc(sizeof(BUTTER));
    b->x_1 = b->x_2 = b->y_1 = b->y_2 = 0;
    b->const0 = M_PI/(double) sr;
    b->lp = lp;
    butter_set_fc(b, 150);
    return b;
}

void butter_next(BUTTER *b, double *in, double *out, int nframes)
{
    while (nframes--) {
        *out = (b->a0 * *in) + (b->a1 * b->x_1) + (b->a2 * b->x_2)
                - (b->b1 * b->y_1) - (b->b2 * b->y_2);
        b->x_2 = b->x_1;
        b->x_1 = *in++;
        b->y_2 = b->y_1;
        b->y_1 = *out++;
    }
}

void butter_free(BUTTER *b)
{
    free(b);
}

/* calculated from table 6.1 in the Audio Programming Book , page 484,
   but there is a typo, so double checked with Richard Dobson's code
   from DVD chapter 2 */
void butter_set_fc(BUTTER *b, double fc)
{
    double lambda;
    if (b->lp) {
        lambda = 1/tan(fc * b->const0);
    } else {
        lambda = tan(fc * b->const0);
    }
    double lambda_2 = lambda * lambda;
    b->a0 = 1.0/(1.0 + (sqrt(2.0)*lambda) + (lambda_2));
    b->a2 = b->a0;
    b->b2 = b->a0 * (1.0 - (sqrt(2.0)*lambda) + (lambda_2));
    if (b->lp) {
        b->a1 = 2.0 * b->a0;
        b->b1 = 2.0 * b->a0 * (1.0 - lambda_2);
    } else {
        b->a1 = - 2.0 * b->a0;
        b->b1 = 2.0 * b->a0 * (lambda_2 - 1.0);
    }
}
