#ifndef BUTTER_H
#define BUTTER_H

typedef struct BUTTER_ BUTTER;

enum {
    BUTTER_HP = 0,
    BUTTER_LP = 1
};

/* lp = 1 for low pass lp = 0 for highpass */
BUTTER * butter_create(int sr, int lp);
void butter_next(BUTTER *b, double *in, double *out, int nframes);
void butter_free(BUTTER *b);

/* not thread safe, must be protected by caller */
void butter_set_fc(BUTTER *b, double fc);

#endif // BUTTER_H
