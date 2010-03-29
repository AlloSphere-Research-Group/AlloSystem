#include "al_quat.hpp"

/*
	C implementation:
*/

typedef al::Quat<double> Quatd;

#define TO_QUAT(q) ((Quatd)(q))
#define TO_QUAT_PTR(q) ((Quatd *)(q))

#define FROM_QUAT(q) ((al_quat)(q))
#define FROM_QUAT_PTR(q) ((al_quat *)(q))

al_quat * al_quat_create(double w, double x, double y, double z) {
	Quatd * qd = new Quatd(w, x, y, z);
	return FROM_QUAT_PTR(qd);
}

void al_quat_free(al_quat ** q) {
	if (q && *q) {
		delete (TO_QUAT_PTR(*q));
		*q = 0;
	}
}

void al_quat_normalize(al_quat * q) {
	(TO_QUAT_PTR(q))->normalize();
}

void al_quat_reset(al_quat * q) {
	(TO_QUAT_PTR(q))->reset();
}

//  TODO etc.
//	void reset();
//	
//	void multiply(Quat * q2);	// in-place
//	void multiply(Quat * q2, Quat * result);
//	void inverse(Quat * result);
//	
//	void fromQuat(T w, T x, T y, T z);
//	void fromAxisAngle(T theta, T x1, T y1, T z1);
//	void fromEuler(T a, T e, T b);
//	
//	void toMatrix(T * matrix);	// 4x4 matrix as T[16]
//	void toAxisAngle(T * aa, T * ax, T * ay, T * az); // axis angle as T[4]
//	void toEuler(T * e);	// euler angles as T[3]
//	void toVectorX(T * x, T * y, T * z);	// vector as T[3]
//	void toVectorY(T * x, T * y, T * z);	// vector as T[3]
//	void toVectorZ(T * x, T * y, T * z);	// vector as T[3]
//	
//	void rotateVector(T * src, T * dst);
//	void rotateVectorTransposed(T * src, T * dst);
//	void rotateby(Quat * q2);	// in-place