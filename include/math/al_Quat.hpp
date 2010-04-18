#ifndef INCLUDE_AL_QUAT_HPP
#define INCLUDE_AL_QUAT_HPP

#include "al_Quat.h"
#include "al_Vec.hpp"

#ifndef ABS
	#define ABS(x) ((x)<0?-(x):(x))
#endif

namespace al {

/*
	Quat<double> is equivalent to struct al_quat
*/

template<typename T=double>
class Quat {	
public:
	
	union{
		struct{
			T w;
			T x;
			T y; 
			T z;
		};
		T components[4];
	};

	Quat(T w = 1.0, T x = 0.0, T y = 0.0, T z = 0.0);
	Quat(const Quat & src);
	
	Quat operator+ (Quat &v) const { Quat q(*this); return q+=v; }
	Quat operator* (const Quat &v) const { return multiply(v); }
	
	Quat& operator  =(const Quat& v){ return set(v); }
	Quat& operator  =(const   T& v){ return set(v); }
	Quat& operator +=(const Quat& v){ w += v.w; x += v.x; y += v.y; z += v.z; return *this;  }
	Quat& operator +=(const   T& v){ w += v; x += v; y += v; z += v; return *this;  }
	Quat& operator -=(const Quat& v){ w -= v.w; x -= v.x; y -= v.y; z -= v.z; return *this;  }
	Quat& operator -=(const   T& v){ w -= v; x -= v; y -= v; z -= v; return *this;  }
	Quat& operator *=(const Quat& v){ set(multiply(v)); return *this; }
	

	Quat& set(T w = (T)1, T x = (T)0, T y = (T)0, T z = (T)0) {
		this->w = w; this->x = x; this->y = y; this->z = z;
		return *this;
	}
	Quat& set(const Quat& q) { w = q.w; x = q.x; y = q.y; z = q.z; return *this; }

	void normalize();
	
	T mag() {
		return (T)sqrt(w*w + x*x + y*y + z*z);
	}
	
	void reset();
	
	Quat<T> multiply(const Quat & q2);
	Quat<T> reverse_multiply(const Quat & q2);
	
	///< in-place multiplication is rotation
	void rotateby(const Quat & q2);	
	
	///< Take the conjugate of a quaternion
	Quat<T> conjugate() {
		return Quat(w, -x, -y, -z);
	}
	
	Quat inverse() {
		// same as conjugate if normalized as q^-1 = q_conj/q_mag^2
		normalize();
		return conjugate();
	}
	
	void fromQuat(T w, T x, T y, T z);
	void fromAxisAngle(T theta, T x1, T y1, T z1);
	void fromEuler(T a, T e, T b);
	void fromMatrix(T * matrix);
	
	void toMatrix(T * matrix);	// 4x4 matrix as T[16]
	void toAxisAngle(T * aa, T * ax, T * ay, T * az); // axis angle as T[4]
	void toEuler(T * e);	// euler angles as T[3]
	void toVectorX(T * x, T * y, T * z);	
	void toVectorY(T * x, T * y, T * z);	
	void toVectorZ(T * x, T * y, T * z);	
	void toVectorX(Vec3<T> & v);	
	void toVectorY(Vec3<T> & v);	
	void toVectorZ(Vec3<T> & v);	
	
	void rotateVector(T * src, T * dst);
	void rotateVector(Vec3<T> & src, Vec3<T> & dst);
	void rotateVectorTransposed(T * src, T * dst);
	void rotateVectorTransposed(Vec3<T> & src, Vec3<T> & dst);
	
	Quat & slerp(Quat &target, T amt) { return slerp(*this, target, amt); }
	static Quat & slerp(Quat &input, Quat &target, T amt);
	
	///< Get the quaternion from a given point and quaterion toward another point
	void toward_point(Vec3<T> &pos, Quat<T> &q, Vec3<T> &v, float amt);
	
	// v1 and v2 must be normalized
	// alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a)))
	static Quat<T> rotor(Vec3<T> &v1, Vec3<T> &v2);

};

typedef Quat<double> Quatd;
typedef Quat<float> Quatf;

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
	w = (T)1.0;		// identity quaternion
	x = y = z = (T)0.0;
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
inline Quat<T> Quat<T> :: multiply(const Quat<T> & q2) {
	Quat q;
	q.w = w*q2.w - x*q2.x - y*q2.y - z*q2.z;
	q.x = w*q2.x + x*q2.w + y*q2.z - z*q2.y;
	q.y = w*q2.y + y*q2.w + z*q2.x - x*q2.z;
	q.z = w*q2.z + z*q2.w + x*q2.y - y*q2.x;
	return q;
}

// assumes both are already normalized!
template<typename T>
inline Quat<T> Quat<T> :: reverse_multiply(const Quat<T> & q2) {
	Quat q;
	return Quat(
		q2.w*w - q2.x*x - q2.y*y - q2.z*z,
		q2.w*x + q2.x*w + q2.y*z - q2.z*y,
		q2.w*y + q2.y*w + q2.z*x - q2.x*z,
		q2.w*z + q2.z*w + q2.x*y - q2.y*x
	);
}

template<typename T>
inline void Quat<T> :: rotateby(const Quat & dq) {
	set(multiply(dq));
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
inline void Quat<T> :: fromMatrix(T *m) {
	T trace = m[0]+m[5]+m[10];
	w = sqrt(1. + trace)*0.5;
	
	if(trace > 0.) {
		x = (m[6] - m[9])/(4.*w);
		y = (m[8] - m[2])/(4.*w);
		z = (m[1] - m[4])/(4.*w);
	}
	else {
		if(m[0] > m[5] && m[0] > m[10]) {
			// m[0] is greatest
			x = sqrt(1. + m[0]-m[5]-m[10])*0.5;
			w = (m[6] - m[9])/(4.*x);
			y = (m[1] + m[4])/(4.*x);
			z = (m[2] + m[8])/(4.*x);
		}
		else if(m[5] > m[0] && m[5] > m[10]) {
			// m[1] is greatest
			y = sqrt(1. + m[5]-m[0]-m[10])*0.5;
			w = (m[8] - m[2])/(4.*y);
			x = (m[1] + m[4])/(4.*y);
			z = (m[6] + m[9])/(4.*y);
		}
		else { //if(m[10] > m[0] && m[10] > m[5]) {
			// m[2] is greatest
			z = sqrt(1. + m[10]-m[0]-m[5])*0.5;
			w = (m[1] - m[4])/(4.*z);
			x = (m[2] + m[8])/(4.*z);
			y = (m[6] + m[9])/(4.*z);
		}
	}
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
inline void Quat<T> :: toVectorX(Vec3<T> & v) {
	v[0] = 1.0 - 2.0*y*y - 2.0*z*z;
	v[1] = 2.0*x*y + 2.0*z*w;
	v[2] = 2.0*x*z - 2.0*y*w;	
}	

template<typename T>
inline void Quat<T> :: toVectorY(Vec3<T> & v) {
	v[0] = 2.0*x*y - 2.0*z*w;
	v[1] = 1.0 - 2.0*x*x - 2.0*z*z;
	v[2] = 2.0*y*z + 2.0*x*w;
}	

template<typename T>
inline void Quat<T> :: toVectorZ(Vec3<T> & v) {
	v[0] = 2.0*x*z + 2.0*y*w;
	v[1] = 2.0*y*z - 2.0*x*w;
	v[2] = 1.0 - 2.0*x*x - 2.0*y*y;
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

template<typename T>
inline void Quat<T> :: rotateVector(Vec3<T> & src, Vec3<T> & dst) {
	T matrix[16];
	toMatrix(matrix);
	dst[0] = src[0] * matrix[0] + src[1] * matrix[1] + src[2] * matrix[2];
	dst[1] = src[0] * matrix[4] + src[1] * matrix[5] + src[2] * matrix[6];
	dst[2] = src[0] * matrix[8] + src[1] * matrix[9] + src[2] * matrix[10];
}

template<typename T>
inline void Quat<T> :: rotateVectorTransposed(Vec3<T> & src, Vec3<T> & dst) {
	T matrix[16];
	toMatrix(matrix);
	dst[0] = src[0] * matrix[0] + src[1] * matrix[4] + src[2] * matrix[8];
	dst[1] = src[0] * matrix[1] + src[1] * matrix[5] + src[2] * matrix[9];
	dst[2] = src[0] * matrix[2] + src[1] * matrix[6] + src[2] * matrix[10];
}

/*!
	Spherical linear interpolation of a quaternion
	Does not change the applied quaternion

	@param result	Resulting interpolated quaternion
	@param target	The quaternion to interpolate toward
	@param amt		The amount to interpolate, range [0, 1]
*/

template<typename T>
Quat<T> & Quat<T> :: slerp(Quat &input, Quat &target, T amt) {
	Quat<T> result;
	int bflip = 0;
	T dot_prod = input.w*target.w + input.x*target.x + input.y*target.y + input.z*target.z;
	T a, b;

	//clamp
	dot_prod = (dot_prod < -1) ? -1 : ((dot_prod > 1) ? 1 : dot_prod);

	// if B is on opposite hemisphere from A, use -B instead
	if (dot_prod < 0.0) {
		dot_prod = -dot_prod;
		bflip = 1;
	}

	T cos_angle = acos(dot_prod);
	if(ABS(cos_angle) > QUAT_EPSILON) {
		T sine = sin(cos_angle);
		T inv_sine = 1./sine;

		a = sin(cos_angle*(1.-amt)) * inv_sine;
		b = sin(cos_angle*amt) * inv_sine;

		if (bflip) {
			b = -b;
		}
	}
	else {
		a = amt;
		b = 1.-amt;
	}

	result.w = a*input.w + b*target.w;
	result.x = a*input.x + b*target.x;
	result.y = a*input.y + b*target.y;
	result.z = a*input.z + b*target.z;

	result.normalize();
	return result;
}

/*!
	Get the quaternion from a given point and quaterion toward another point
*/
template<typename T>
void Quat<T> :: toward_point(Vec3<T> &pos, Quat<T> &q, Vec3<T> &v, float amt) {
	Vec3<T> diff, axis;
	Vec3<T>::sub(diff, v, pos);
	Vec3<T>::normalize(diff);
	
	if(amt < 0) {
		diff = diff*-1.;
	}
	
	Vec3<T> zaxis;
	q.toVectorZ(zaxis);
	Vec3<T>::cross(axis, zaxis, diff);
	Vec3<T>::normalize(axis);
	
	float axis_mag_sqr = Vec3<T>::dot(axis, axis);
	float along = Vec3<T>::dot(zaxis, diff);
	
	if(axis_mag_sqr < 0.001 && along < 0) {
		Vec3<T>::cross(axis, zaxis, Vec3<T>(0., 0., 1.));
		Vec3<T>::normalize(axis);
		
		if(axis_mag_sqr < 0.001) {
			Vec3<T>::cross(axis, zaxis, Vec3<T>(0., 1., 0.));
			Vec3<T>::normalize(axis);
		}
		
		axis_mag_sqr = Vec3<T>::dot(axis, axis);
	}
	
	if(along < 0.9995 && axis_mag_sqr > 0.001) {
		float theta = ABS(amt)*acos(along)*QUAT_RAD2DEG;
//			printf("theta: %f  amt: %f\n", theta, amt);
		fromAxisAngle(theta, axis.x, axis.y, axis.z);
	}
	else {
		reset();
	}
}

// v1 and v2 must be normalized
// alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a)))
template<typename T>
Quat<T> Quat<T> :: rotor(Vec3<T> &v1, Vec3<T> &v2) {
	// get the normal to the plane (i.e. the unit bivector containing the v1 and v2)
	Vec3<T> n;
	Vec3<T>::cross(n, v1, v2);
	Vec3<T>::normalize(n);	// normalize because the cross product can get slightly denormalized

	// calculate half the angle between v1 and v2
	T dotmag = Vec3<T>::dot(v1, v2);
	T theta = acos(dotmag)*0.5;

	// calculate the scaled actual bivector generaed by v1 and v2
	Vec3<T> bivec = n*sin(theta);
	Quat<T> q(cos(theta), bivec.x, bivec.y, bivec.z);

	return q;
}

} // namespace

#endif /* include guard */
