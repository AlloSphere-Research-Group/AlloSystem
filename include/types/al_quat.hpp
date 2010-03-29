#ifndef INCLUDE_AL_QUAT_HPP
#define INCLUDE_AL_QUAT_HPP

#include "al_quat.h"

namespace al {

template<typename T=double>
class Quat {	
public:
	T w;
	T x;
	T y; 
	T z;

	Quat(T w = 1.0, T x = 0.0, T y = 0.0, T z = 0.0);
	Quat(const Quat & src);

	void normalize();
	void reset();
	
	void multiply(Quat * q2);	// in-place
	void multiply(Quat * q2, Quat * result);
	void inverse(Quat * result);
	
	void fromQuat(T w, T x, T y, T z);
	void fromAxisAngle(T theta, T x1, T y1, T z1);
	void fromEuler(T a, T e, T b);
	
	void toMatrix(T * matrix);	// 4x4 matrix as T[16]
	void toAxisAngle(T * aa, T * ax, T * ay, T * az); // axis angle as T[4]
	void toEuler(T * e);	// euler angles as T[3]
	void toVectorX(T * x, T * y, T * z);	// vector as T[3]
	void toVectorY(T * x, T * y, T * z);	// vector as T[3]
	void toVectorZ(T * x, T * y, T * z);	// vector as T[3]
	
	void rotateVector(T * src, T * dst);
	void rotateVectorTransposed(T * src, T * dst);
	void rotateby(Quat * q2);	// in-place
};

template<typename T>
inline Quat<T> :: Quat(T w, T x, T y, T z)
: w(w), x(x), y(y), z(z)
{}

template<typename T>
inline Quat<T> :: Quat(const Quat & src) {
	w = src.w; x = src.x; y = src.y; z = src.z;
}

template<typename T>
inline void Quat<T> :: reset() {
	w = 1.0;		// identity quaternion
	x = y = z = 0.0;
}

template<typename T>
inline void Quat<T> :: normalize() {
	T unit = w*w + x*x + y*y + z*z;
	if (unit*unit < QUAT_EPSILON) { 
		// unit too close to epsilon, set to default transform
		w = 1.0; 
		x = y = z = 0.0;
		return;
	}
	if (unit > QUAT_ACCURACY_MAX || unit < QUAT_ACCURACY_MIN) {
		T invmag = 1.0/sqrt(unit);
		w *= invmag; 
		x *= invmag; 
		y *= invmag;
		z *= invmag;
	}
}

// assumes both are already normalized!
template<typename T>
inline void Quat<T> :: multiply(Quat * q1, Quat * q2) {
	w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
	x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
	y = q1->w*q2->y + q1->y*q2->w + q1->z*q2->x - q1->x*q2->z;
	z = q1->w*q2->z + q1->z*q2->w + q1->x*q2->y - q1->y*q2->x;
}

template<typename T>
inline void Quat<T> :: multiply(Quat * q1) {
	Quat * q = new Quat(*this);
	multiply(q1, q);
	delete q;
}

template<typename T>
inline void Quat<T> :: rotateby(Quat * dq) {
	Quat * q = new Quat(*this);
	multiply(q, dq);
	delete q;
}

template<typename T>
inline void Quat<T> :: inverse(Quat * result) {
	normalize();	
	result->w = w;
	result->x = -x;
	result->y = -y;
	result->z = -z;
}

template<typename T>
inline void Quat<T> :: fromQuat(T w, T x, T y, T z)
{
	this->w = w; this->x = x; this->y = y; this->z = z;
}

template<typename T>
inline void Quat<T> :: fromAxisAngle(T theta, T x1, T y1, T z1) {
	T t2 = theta * 0.00872664626; // * 0.5 * 180/PI
	T sinft2 = sin(t2);
	w = cos(t2);
	x = x1 * sinft2;
	y = y1 * sinft2;
	z = z1 * sinft2;
	normalize();
}

template<typename T>
inline void Quat<T> :: fromEuler(T az, T el, T ba) {
	//http://vered.rose.utoronto.ca/people/david_dir/GEMS/GEMS.html
	//Converting from Euler angles to a quaternion is slightly more tricky, as the order of operations 
	//must be correct. Since you can convert the Euler angles to three independent quaternions by 
	//setting the arbitrary axis to the coordinate axes, you can then multiply the three quaternions 
	//together to obtain the final quaternion.

	//So if you have three Euler angles (a, b, c), then you can form three independent quaternions
	//Qx = [ cos(a/2), (sin(a/2), 0, 0)]
    //Qy = [ cos(b/2), (0, sin(b/2), 0)]
    //Qz = [ cos(c/2), (0, 0, sin(c/2))]
	//And the final quaternion is obtained by Qx * Qy * Qz.
	
	T c1 = cos(az * QUAT_DEG2RAD_BY2);
	T c2 = cos(el * QUAT_DEG2RAD_BY2);
	T c3 = cos(ba * QUAT_DEG2RAD_BY2);
	T s1 = sin(az * QUAT_DEG2RAD_BY2);
	T s2 = sin(el * QUAT_DEG2RAD_BY2);
	T s3 = sin(ba * QUAT_DEG2RAD_BY2);

	// equiv quat_multiply(&Qy, &Qx, &Q1); // since many terms are zero
	T tw = c1*c2;
	T tx = c1*s2;
	T ty = s1*c2;
	T tz = - s1*s2;
	
	// equiv quat_multiply(&Q1, &Qz, &Q2); // since many terms are zero
	w = tw*c3 - tz*s3;
	x = tx*c3 + ty*s3;
	y = ty*c3 - tx*s3;
	z = tw*s3 + tz*c3;
	normalize();
}

template<typename T>
inline void Quat<T> :: toMatrix(T * m) {
	m[0] = 1.0 - 2.0*y*y - 2.0*z*z;
	m[1] = 2.0*x*y - 2.0*z*w;
	m[2] = 2.0*x*z + 2.0*y*w;
	m[3] = 0.0;
	
	m[4] = 2.0*x*y + 2.0*z*w;
	m[5] = 1.0 - 2.0*x*x - 2.0*z*z;
	m[6] = 2.0*y*z - 2.0*x*w;
	m[7] = 0.0;
	
	m[8] = 2.0*x*z - 2.0*y*w;
	m[9] = 2.0*y*z + 2.0*x*w;
	m[10] = 1.0 - 2.0*x*x - 2.0*y*y;
	m[11] = 0.0;
	
	m[12] = m[13] = m[14] = 0.0;
	m[15] = 1.0;
}

template<typename T>
inline void Quat<T> :: toAxisAngle(T * aa, T * ax, T * ay, T * az) {
  T unit = w*w;
  if (unit > QUAT_ACCURACY_MAX || unit < QUAT_ACCURACY_MIN) {
	T invSinAngle = 1.f/sqrt(1.f - unit);
	*aa = acosf(w) * 114.59155902616; // * 2 * 180 / pi
	*ax = x * invSinAngle;
	*ay = y * invSinAngle;
	*az = z * invSinAngle;
  } else {
	*aa = 0.f;
	*ax = x;
	*ay = y;
	*az = z;
  }
}

template<typename T>
inline void Quat<T> :: toEuler(T * e) {
	// http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/quaternionstoeulerangles.html
	T sqw = w*w;
	T sqx = x*x;
	T sqy = y*y;
	T sqz = z*z;
	e[0] = QUAT_RAD2DEG * asin(-2.0 * (x*z - w*y));
	e[1] = QUAT_RAD2DEG * atan2(2.0 * (y*z + w*x),(sqw - sqx - sqy + sqz));
	e[3] = QUAT_RAD2DEG * atan2(2.0 * (x*y + w*z), (sqw + sqx - sqy - sqz));
}

template<typename T>
inline void Quat<T> :: toVectorX(T * vx, T * vy, T * vz) {
	*vx = 1.0 - 2.0*y*y - 2.0*z*z;
	*vy = 2.0*x*y + 2.0*z*w;
	*vz = 2.0*x*z - 2.0*y*w;	
}

template<typename T>
inline void Quat<T> :: toVectorY(T * vx, T * vy, T * vz) {
	*vx = 2.0*x*y - 2.0*z*w;
	*vy = 1.0 - 2.0*x*x - 2.0*z*z;
	*vz = 2.0*y*z + 2.0*x*w;
}

template<typename T>
inline void Quat<T> :: toVectorZ(T * vx, T * vy, T * vz) {
	*vx = 2.0*x*z + 2.0*y*w;
	*vy = 2.0*y*z - 2.0*x*w;
	*vz = 1.0 - 2.0*x*x - 2.0*y*y;
}

template<typename T>
inline void Quat<T> :: rotateVector(T * src, T * dst) {
	T matrix[16];
	toMatrix(matrix);
	dst[0] = src[0] * matrix[0] + src[1] * matrix[1] + src[2] * matrix[2];
	dst[1] = src[0] * matrix[4] + src[1] * matrix[5] + src[2] * matrix[6];
	dst[2] = src[0] * matrix[8] + src[1] * matrix[9] + src[2] * matrix[10];
}

template<typename T>
inline void Quat<T> :: rotateVectorTransposed(T * src, T * dst) {
	T matrix[16];
	toMatrix(matrix);
	dst[0] = src[0] * matrix[0] + src[1] * matrix[4] + src[2] * matrix[8];
	dst[1] = src[0] * matrix[1] + src[1] * matrix[5] + src[2] * matrix[9];
	dst[2] = src[0] * matrix[2] + src[1] * matrix[6] + src[2] * matrix[10];
}

} // namespace

#endif /* include guard */