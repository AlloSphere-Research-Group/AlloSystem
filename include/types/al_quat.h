#ifndef INCLUDE_AL_QUAT_H
#define INCLUDE_AL_QUAT_H

#include <math.h>

#define QUAT_ACCURACY_MAX (1.000001)
#define QUAT_ACCURACY_MIN (0.999999)
#define QUAT_EPSILON (0.000001)
#define QUAT_RAD2DEG (57.29577951308)
#define QUAT_DEG2RAD (0.01745329252)
#define QUAT_DEG2RAD_BY2 (0.00872664626)
#define QUAT_PIBY2 (1.57079632679)

typedef struct al_quat {
	double w;
	double x;
	double y;
	double z;
} al_quat;

al_quat * al_quat_create(double w, double x, double y, double z);
void al_quat_free(al_quat ** q);
void al_quat_normalize(al_quat * q);
void al_quat_reset(al_quat * q);

#endif /* include guard */