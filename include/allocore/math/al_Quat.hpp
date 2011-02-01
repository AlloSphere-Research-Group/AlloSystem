#ifndef INCLUDE_AL_QUAT_HPP
#define INCLUDE_AL_QUAT_HPP

#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Vec.hpp"

#define QUAT_ACCURACY_MAX (1.000001)
#define QUAT_ACCURACY_MIN (0.999999)
#define QUAT_EPSILON (0.0000001)
//#define QUAT_DEG2RAD_BY2 (M_DEG2RAD/2.)

#ifndef ABS
	#define ABS(x) ((x)<0?-(x):(x))
#endif

namespace al {


template<class T> class Quat;

typedef Quat<float>		Quatf;	///< Single-precision quaternion
typedef Quat<double>	Quatd;	///< Double-precision quaternion


/// Quaternion
template<typename T=double>
class Quat {
public:
	union{
		struct{
			T w;			///< w component
			T x;			///< x component
			T y;			///< y component
			T z;			///< z component
		};
		T components[4];	///< component vector
	};
	
	///
	Quat(const T& w = T(1), const T& x = T(0), const T& y = T(0), const T& z = T(0));

	/// @param[in] src		quaternion to set values from
	Quat(const Quat& src);
	
	/// constructor of 'pure' quaternion
	Quat(const Vec3<T>& v) { w=0; x=v[0]; y=v[1]; z=v[2]; }

	/// Set component with index
	T& operator[](int i){ return components[i];}

	/// Get component with index
	const T& operator[](int i) const { return components[i]; }

	/// Returns true if all components are equal
	bool operator ==(const Quat& v) const { return (w==v.w) && (x==v.x) && (y==v.y) && (z==v.z); }

	/// Returns true if any components are not equal
	bool operator !=(const Quat& v) const {  return !(*this == v); }

	Quat operator - () const { return Quat(-w, -x, -y, -z); }
	Quat operator + (const Quat& v) const { return Quat(*this)+=v; }
	Quat operator + (const    T& v) const { return Quat(*this)+=v; }
	Quat operator - (const Quat& v) const { return Quat(*this)-=v; }
	Quat operator - (const    T& v) const { return Quat(*this)-=v; }
	Quat operator / (const Quat& v) const { return Quat(*this)/=v; }
	Quat operator / (const    T& v) const { return Quat(*this)/=v; }
	Quat operator * (const Quat& v) const { return Quat(*this)*=v; }
	Vec3<3,T> operator * (const Vec3<3,T>& v) const { return rotateVectorTransposed(v); }
	Quat operator * (const    T& v) const { return Quat(*this)*=v; }


	Quat& operator  =(const Quat& v){ return set(v); }
	Quat& operator  =(const    T& v){ return set(v); }
	Quat& operator +=(const Quat& v){ w+=v.w; x+=v.x; y+=v.y; z+=v.z; return *this; }
	Quat& operator +=(const    T& v){ w+=  v; x+=  v; y+=  v; z+=  v; return *this; }
	Quat& operator -=(const Quat& v){ w-=v.w; x-=v.x; y-=v.y; z-=v.z; return *this; }
	Quat& operator -=(const    T& v){ w-=  v; x-=  v; y-=  v; z-=  v; return *this; }
	Quat& operator *=(const Quat& v){ return set(multiply(v)); }
	Quat& operator *=(const    T& v){ w*=  v; x*=  v; y*=  v; z*=  v; return *this; }
	Quat& operator /=(const Quat& v){ return (*this) *= v.recip(); }
	Quat& operator /=(const    T& v){ w/=v; x/=v; y/=v; z/=v; return *this; }


	/// Returns the conjugate
	Quat conj() const { return Quat(w, -x, -y, -z); }

	/// Returns dot product with another quaternion
	T dot(const Quat& v) const { return w*v.w + x*v.x + y*v.y + z*v.z; }

	/// Returns identity
	static Quat identity(){ return Quat(1,0,0,0); }

	/// Returns inverse (same as conjugate if normalized as q^-1 = q_conj/q_mag^2)
	Quat inverse() const { return sgn().conj(); }

	/// Get magnitude
	T mag() const { return (T)sqrt(magSqr()); }

	/// Get magnitude squared
	T magSqr() const { return dot(*this); }

	/// Returns multiplicative inverse
	Quat recip() const { return conj()/magSqr(); }

	/// Returns signum, q/|q|, the closest point on unit 3-sphere
	Quat sgn() const { return Quat(*this).normalize(); }

	Quat multiply(const Quat & q2) const;
	Quat reverseMultiply(const Quat & q2) const;

	/// Normalize magnitude to one
	Quat& normalize();

	/// Set components
	Quat& set(const T& w, const T& x, const T& y, const T& z){
		this->w = w; this->x = x; this->y = y; this->z = z;
		return *this;
	}

	/// Set from other quaternion
	Quat& set(const Quat& q){ return set(q.w, q.x, q.y, q.z); }

	/// Set to identity
	Quat& setIdentity(){ return (*this) = Quat::identity(); }

	/// Set as versor rotated by angle around unit vector
	Quat& fromAxisAngle(T angle, T x1, T y1, T z1);

	/// Set as versor rotated by angle around unit vector
	Quat& fromAxisAngle(T angle, const Vec<3,T>& axis) { return fromAxisAngle(angle, axis[0], axis[1], axis[2]); }

	/// Set as versor rotated by angle around x-axis
	Quat& fromAxisX(T angle) {
		T t2 = angle * degToHalfRad();
		return set(cos(t2), sin(t2), T(0), T(0));
	}

	/// Set as versor rotated by angle around y-axis
	Quat& fromAxisY(T angle){
		T t2 = angle * degToHalfRad();
		return set(cos(t2), T(0), sin(t2), T(0));
	}

	/// Set as versor rotated by angle around z-axis
	Quat& fromAxisZ(T angle){
		T t2 = angle * degToHalfRad();
		return set(cos(t2), T(0), T(0), sin(t2));
	}

	/// Set as versor rotated by Euler angles, in degrees
	Quat& fromEuler(T az, T el, T ba);
	
	/// Set as versor from column-major 4-by-4 projective space transformation matrix
	Quat& fromMatrix(const T * matrix);

	/// Set as versor from column-major 4-by-4 projective space transformation matrix
	Quat& fromMatrix(const Mat<4,T>& v){ return fromMatrix(v.ptr()); }

	/// Set as versor from row-major 4-by-4 projective space transformation matrix
	Quat& fromMatrixTransposed(const T * matrix);

	/// Set as versor from row-major 4-by-4 projective space transformation matrix
	Quat& fromMatrixTransposed(const Mat<4,T>& v){ return fromMatrixTransposed(v.ptr()); }


	/// Convert to coordinate frame unit vectors
	void toCoordinateFrame(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const;

	/// Convert to column-major 4-by-4 projective space transformation matrix
	void toMatrix(T * matrix) const;

	/// Convert to row-major 4-by-4 projective space transformation matrix
	void toMatrixTransposed(T * matrix) const;

	/// Convert to axis-angle form
	void toAxisAngle(T& angle, T& ax, T& ay, T& az) const;

	/// Convert to Euler angles (azimuth, elevation, bank), in degrees
	void toEuler(T& az, T& el, T& ba) const;

	/// Convert to Euler angle vector (azimuth, elevation, bank), in degrees
	void toEuler(T * e) const { toEuler(e[0],e[1],e[2]); }

	/// Convert to Euler angle vector (azimuth, elevation, bank), in degrees
	void toEuler(Vec<3,T> & v) const { toEuler(v[0],v[1],v[2]); }

	/// Get local x unit vector (1,0,0) in absolute coordinates
	void toVectorX(T& ax, T& ay, T& az) const;

	/// Get local y unit vector (0,1,0) in absolute coordinates
	void toVectorY(T& ax, T& ay, T& az) const;

	/// Get local z unit vector (0,0,1) in absolute coordinates
	void toVectorZ(T& ax, T& ay, T& az) const;

	/// Get local x unit vector (1,0,0) in absolute coordinates
	void toVectorX(Vec<3,T>& v) const { toVectorX(v[0],v[1],v[2]); }
	
	/// Get local y unit vector (0,1,0) in absolute coordinates
	void toVectorY(Vec<3,T>& v) const { toVectorY(v[0],v[1],v[2]); }
	
	/// Get local z unit vector (0,0,1) in absolute coordinates
	void toVectorZ(Vec<3,T>& v) const { toVectorZ(v[0],v[1],v[2]); }

	/// Rotate vector
	void rotate(Vec<3,T>& v) const;
	void rotateVector(const Vec<3,T>& src, Vec<3,T>& dst) const;
	Vec<3,T> rotateVector(const Vec<3,T>& src) const;
	void rotateVectorTransposed(const T * src, T * dst) const;
	Vec<3,T> rotateVectorTransposed(const Vec<3,T>& src) const;

	/// Spherical interpolation
	Quat& slerp(const Quat& target, T amt) { return set(slerp(*this, target, amt)); }

	/// Fill an array of Quats with a full spherical interpolation:
	static void slerp_buffer(const Quat& input, const Quat& target, Quat<T> * buffer, int numFrames);

	///	Spherical linear interpolation of a quaternion

	///	@param[in] result	Resulting interpolated quaternion
	///	@param[in] target	The quaternion to interpolate toward
	///	@param[in] amt		The amount to interpolate, range [0, 1]
	static Quat slerp(const Quat& input, const Quat& target, T amt);

	/// Get the quaternion from a given point and quaterion toward another point
	void towardPoint(const Vec<3,T>& pos, const Quat<T>& q, const Vec<3,T>& v, float amt);


	/// Get rotor from two unit vectors
	
	/// Alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a))).
	///
	static Quat<T> rotor(const Vec<3,T>& v1, const Vec<3,T>& v2);
	
	
	///! calculate the rotation required to move from unit vector src to unit vector dst
	/// rotation occurs around the axis created by the cross product of src and dst
	/// if the vectors are nearly opposing, the Y axis is used instead 
	/// if the Y axis isn't suitable, the Z axis is used instead
	/// 
	/// a typical use case: rotate object A to face object B:
	/// Vec3d src = Vec3d(A.quat().toVectorZ()).normalize();
	/// Vec3d dst = Vec3d(B.pos() - A.pos()).normalize();
	/// Quatd rot = Quatd::getRotationTo(src, dst);
	/// A.quat() = rot * A.quat();
	static Quat<T> getRotationTo(const Vec<3, T>& src, const Vec<3, T>& dst);
	
	/// these are 180 degree rotations around the various axes:
	void flipX() { return set(T(0), T(1), T(0), T(0)); } 
	void flipY() { return set(T(0), T(0), T(1), T(0)); } 
	void flipZ() { return set(T(0), T(0), T(0), T(1)); } 
	
protected:
	static const double degToHalfRad(){ return 0.5*M_DEG2RAD; }
	static const double halfRadToDeg(){ return 2.0*M_RAD2DEG; }
};

template<class T> Quat<T> operator + (T r, const Quat<T>& q){ return  q+r; }
template<class T> Quat<T> operator - (T r, const Quat<T>& q){ return -q+r; }
template<class T> Quat<T> operator * (T r, const Quat<T>& q){ return  q*r; }
template<class T> Quat<T> operator / (T r, const Quat<T>& q){ return  q.conjugate()*(r/q.magSqr()); }





/// Implementation

template<typename T>
inline Quat<T> :: Quat(const T& w, const T& x, const T& y, const T& z)
: w(w), x(x), y(y), z(z)
{}

template<typename T>
inline Quat<T> :: Quat(const Quat& q) {
	w = q.w; x = q.x; y = q.y; z = q.z;
}

template<typename T>
inline Quat<T>& Quat<T> :: normalize() {
	T unit = magSqr();
	if(unit*unit < QUAT_EPSILON){
		// unit too close to epsilon, set to default transform
		setIdentity();
	}
	else if(unit > QUAT_ACCURACY_MAX || unit < QUAT_ACCURACY_MIN){
		(*this) *= 1./sqrt(unit);
	}
	return *this;
}

// assumes both are already normalized!
template<typename T>
inline Quat<T> Quat<T> :: multiply(const Quat<T>& q) const {
	return Quat(
		w*q.w - x*q.x - y*q.y - z*q.z,
		w*q.x + x*q.w + y*q.z - z*q.y,
		w*q.y + y*q.w + z*q.x - x*q.z,
		w*q.z + z*q.w + x*q.y - y*q.x
	);
}

// assumes both are already normalized!
template<typename T>
inline Quat<T> Quat<T> :: reverseMultiply(const Quat<T> & q) const {
	return q * (*this);
}

template<typename T>
inline Quat<T>& Quat<T> :: fromAxisAngle(T angle, T x1, T y1, T z1) {
	T t2 = angle * degToHalfRad();
	T sinft2 = sin(t2);
	w = cos(t2);
	x = x1 * sinft2;
	y = y1 * sinft2;
	z = z1 * sinft2;
	return normalize();
}

template<typename T>
inline Quat<T>& Quat<T>::fromEuler(T az, T el, T ba){
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

	T c1 = cos(az * degToHalfRad());
	T c2 = cos(el * degToHalfRad());
	T c3 = cos(ba * degToHalfRad());
	T s1 = sin(az * degToHalfRad());
	T s2 = sin(el * degToHalfRad());
	T s3 = sin(ba * degToHalfRad());

	// equiv Q1 = Qy * Qx; // since many terms are zero
	T tw = c1*c2;
	T tx = c1*s2;
	T ty = s1*c2;
	T tz =-s1*s2;

	// equiv Q2 = Q1 * Qz; // since many terms are zero
	w = tw*c3 - tz*s3;
	x = tx*c3 + ty*s3;
	y = ty*c3 - tx*s3;
	z = tw*s3 + tz*c3;
	return normalize();
}

template<typename T>
inline Quat<T>& Quat<T> :: fromMatrix(const T * m) {
	T trace = m[0]+m[5]+m[10];
	w = sqrt(1. + trace)*0.5;

	if(trace > 0.) {
		x = (m[9] - m[6])/(4.*w);
		y = (m[2] - m[8])/(4.*w);
		z = (m[4] - m[1])/(4.*w);
	}
	else {
		if(m[0] > m[5] && m[0] > m[10]) {
			// m[0] is greatest
			x = sqrt(1. + m[0]-m[5]-m[10])*0.5;
			w = (m[9] - m[6])/(4.*x);
			y = (m[4] + m[1])/(4.*x);
			z = (m[8] + m[2])/(4.*x);
		}
		else if(m[5] > m[0] && m[5] > m[10]) {
			// m[1] is greatest
			y = sqrt(1. + m[5]-m[0]-m[10])*0.5;
			w = (m[2] - m[8])/(4.*y);
			x = (m[4] + m[1])/(4.*y);
			z = (m[9] + m[6])/(4.*y);
		}
		else { //if(m[10] > m[0] && m[10] > m[5]) {
			// m[2] is greatest
			z = sqrt(1. + m[10]-m[0]-m[5])*0.5;
			w = (m[4] - m[1])/(4.*z);
			x = (m[8] + m[2])/(4.*z);
			y = (m[9] + m[6])/(4.*z);
		}
	}
	return *this;
}

template<typename T>
inline Quat<T>& Quat<T> :: fromMatrixTransposed(const T * m) {
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
	return *this;
}


template<typename T>
inline void Quat<T> :: toAxisAngle(T& aa, T& ax, T& ay, T& az) const {
	T unit = w*w;
	if(unit > QUAT_ACCURACY_MAX || unit < QUAT_ACCURACY_MIN){
		T invSinAngle = 1.f/sqrt(1.f - unit);
		aa = acosf(w) * halfRadToDeg();
		ax = x * invSinAngle;
		ay = y * invSinAngle;
		az = z * invSinAngle;
	} else {
		aa = 0.f;
		ax = x;
		ay = y;
		az = z;
	}
}

template<typename T>
inline void Quat<T> :: toEuler(T& az, T& el, T& ba) const {
	// http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/quaternionstoeulerangles.html
	T sqw = w*w;
	T sqx = x*x;
	T sqy = y*y;
	T sqz = z*z;
	az = M_RAD2DEG * asin (-2.0 * (x*z - w*y));
	el = M_RAD2DEG * atan2( 2.0 * (y*z + w*x), (sqw - sqx - sqy + sqz));
	ba = M_RAD2DEG * atan2( 2.0 * (x*y + w*z), (sqw + sqx - sqy - sqz));
}

template<typename T>
inline void Quat<T>::toCoordinateFrame(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const {
	static const T _2 = T(2);
	static const T _1 = T(1);
	T	_2w=_2*w, _2x=_2*x, _2y=_2*y;
	T	wx=_2w*x, wy=_2w*y, wz=_2w*z, xx=_2x*x, xy=_2x*y,
		xz=_2x*z, yy=_2y*y, yz=_2y*z, zz=_2*z*z;

	ux[0] =-zz - yy + _1;
	ux[1] = wz + xy;
	ux[2] = xz - wy;

	uy[0] = xy - wz;
	uy[1] =-zz - xx + _1;
	uy[2] = wx + yz;

	uz[0] = wy + xz;
	uz[1] = yz - wx;
	uz[2] =-yy - xx + _1;
}

template<typename T>
inline void Quat<T> :: toMatrix(T * m) const {
	Vec<3,T> ux,uy,uz;
	toCoordinateFrame(ux,uy,uz);

	m[ 0] = ux[0];	m[ 4] = uy[0];	m[ 8] = uz[0];	m[12] = 0;
	m[ 1] = ux[1];	m[ 5] = uy[1];	m[ 9] = uz[1];	m[13] = 0;
	m[ 2] = ux[2];	m[ 6] = uy[2];	m[10] = uz[2];	m[14] = 0;
	m[ 3] = 0;		m[ 7] = 0;		m[11] = 0;		m[15] = 1;
}

// Note: same as toMatrix, but with matrix indices transposed
template<typename T>
inline void Quat<T> :: toMatrixTransposed(T * m) const {
	Vec<3,T> ux,uy,uz;
	toCoordinateFrame(ux,uy,uz);

	m[ 0] = ux[0];	m[ 1] = uy[0];	m[ 2] = uz[0];	m[ 3] = 0;
	m[ 4] = ux[1];	m[ 5] = uy[1];	m[ 6] = uz[1];	m[ 7] = 0;
	m[ 8] = ux[2];	m[ 9] = uy[2];	m[10] = uz[2];	m[11] = 0;
	m[12] = 0;		m[13] = 0;		m[14] = 0;		m[15] = 1;
}

template<typename T>
inline void Quat<T> :: toVectorX(T& ax, T& ay, T& az) const {
	ax = 1.0 - 2.0*y*y - 2.0*z*z;
	ay = 2.0*x*y + 2.0*z*w;
	az = 2.0*x*z - 2.0*y*w;
}

template<typename T>
inline void Quat<T> :: toVectorY(T& ax, T& ay, T& az) const {
	ax = 2.0*x*y - 2.0*z*w;
	ay = 1.0 - 2.0*x*x - 2.0*z*z;
	az = 2.0*y*z + 2.0*x*w;
}

template<typename T>
inline void Quat<T> :: toVectorZ(T& ax, T& ay, T& az) const {
	ax = 2.0*x*z + 2.0*y*w;
	ay = 2.0*y*z - 2.0*x*w;
	az = 1.0 - 2.0*x*x - 2.0*y*y;
}

template<typename T>
inline void Quat<T>::rotate(Vec<3,T>& v) const{ rotateVector(v,v); }

/*
	Rotating a vector should be simpler:
	
	v1 = q * qv * q^-1
	
	Where v is a 'pure quaternion' derived from the vector, i.e. w = 0. 	
*/
template<typename T>
inline void Quat<T> :: rotateVector(const Vec<3,T>& src, Vec<3,T>& dst) const {
	Quat q(*this);
	q.normalize();
	Quat qi = q.conj();
	Quat qv(src);	// pure quat
	Quat qr = q * qv * qi;
	dst.set(qr.x, qr.y, qr.z);
}
//template<typename T>
//inline void Quat<T> :: rotateVector(const Vec<3,T>& src, Vec<3,T>& dst) const {
//	static const T c1 = T(1);
//	static const T c2 = T(2);
//	const T x = src[0];
//	const T y = src[1];
//	const T z = src[2];
//	const T x2 = x*x;
//	const T y2 = y*y;
//	const T z2 = z*z;
//	const T xy = x*y;
//	const T xz = x*z;
//	const T yz = y*z;
//	const T xw = x*w;
//	const T yw = y*w;
//	const T zw = z*w;
//	// unit vectors of quaternion:
//	const T ux[3] = {
//		c1 - c2*y2 - c2*z2,
//		c2*xy + c2*zw,
//		c2*xz - c2*yw
//	};
//	const T uy[3] = {
//		c2*xy - c2*zw,
//		c1 - c2*x2 - c2*z2,
//		c2*yz + c2*xw
//	};
//	const T uz[3] = {
//		c2*xz + c2*yw,
//		c2*yz - c2*xw,
//		c1 - c2*x2 - c2*y2
//	};
//	// matrix multiply:
//	dst[0] = src[0] * ux[0] + src[1] * ux[1] + src[2] * ux[2];
//	dst[1] = src[0] * uy[0] + src[1] * uy[1] + src[2] * uy[2];
//	dst[2] = src[0] * uz[0] + src[1] * uz[1] + src[2] * uz[2];
//}

template<typename T>
inline Vec<3,T> Quat<T> :: rotateVector(const Vec<3,T>& src) const {
	Vec<3,T> dst;
	rotateVector(&src[0], &dst[0]);
	return dst;
}

template<typename T>
inline void Quat<T> :: rotateVectorTransposed(const Vec<3,T>& src, Vec<3,T>& dst) const {
	Quat q(*this);
	q.normalize();
	Quat qi = q.conj();
	Quat qv(src);	// pure quat
	Quat qr = qi * qv * q;
	dst.set(qr.x, qr.y, qr.z);
}
//template<typename T>
//inline void Quat<T> :: rotateVectorTransposed(const T * src, T * dst) const {
//	static const T c1 = T(1);
//	static const T c2 = T(2);
//	const T x = src[0];
//	const T y = src[1];
//	const T z = src[2];
//	const T x2 = x*x;
//	const T y2 = y*y;
//	const T z2 = z*z;
//	const T xy = x*y;
//	const T xz = x*z;
//	const T yz = y*z;
//	const T xw = x*w;
//	const T yw = y*w;
//	const T zw = z*w;
//	// unit vectors of quaternion:
//	const T ux[3] = {
//		c1 - c2*y2 - c2*z2,
//		c2*xy + c2*zw,
//		c2*xz - c2*yw
//	};
//	const T uy[3] = {
//		c2*xy - c2*zw,
//		c1 - c2*x2 - c2*z2,
//		c2*yz + c2*xw
//	};
//	const T uz[3] = {
//		c2*xz + c2*yw,
//		c2*yz - c2*xw,
//		c1 - c2*x2 - c2*y2
//	};
//	// matrix multiply:
//	dst[0] = src[0] * ux[0] + src[1] * uy[0] + src[2] * uz[0];
//	dst[1] = src[0] * ux[1] + src[1] * uy[1] + src[2] * uz[1];
//	dst[2] = src[0] * ux[2] + src[1] * uy[2] + src[2] * uz[2];
//}

template<typename T>
inline Vec<3,T> Quat<T> :: rotateVectorTransposed(const Vec<3,T>& src) const {
	Vec<3,T> dst;
	rotateVectorTransposed(&src[0], &dst[0]);
	return dst;
}


template<typename T>
Quat<T> Quat<T> :: slerp(const Quat& input, const Quat& target, T amt){
	Quat<T> result;
	int bflip = 0;
	T dot_prod = input.dot(target);
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

template<typename T>
void Quat<T> :: slerp_buffer(const Quat& input, const Quat& target, Quat<T> * buffer, int numFrames){


	/// Sinusoidal generator based on recursive formula x0 = c x1 - x2
	struct RSin {

		/// Constructor
		RSin(const T& frq=T(0), const T& phs=T(0), const T& amp=T(1))
		:	val2(0), mul(0){
			T f=frq, p=phs;
			mul  = (T)2 * (T)cos(f);
			val2 = (T)sin(p - f * T(2))*amp;
			val  = (T)sin(p - f       )*amp;
		}

		/// Generate next value.
		T operator()() const {
			T v0 = mul * val - val2;
			val2 = val;
			return val = v0;
		}

		mutable T val;
		mutable T val2;
		T mul;			///< Multiplication factor. [-2, 2] range gives stable sinusoids.
	};


	int bflip = 1;
	T dot_prod = input.dot(target);

	//clamp
	dot_prod = (dot_prod < -1) ? -1 : ((dot_prod > 1) ? 1 : dot_prod);

	// if B is on opposite hemisphere from A, use -B instead
	if (dot_prod < 0.0) {
		dot_prod = -dot_prod;
		bflip = -1;
	}

	const T cos_angle = acos(dot_prod);
	const T inv_frames = 1./((T)numFrames);

	if(ABS(cos_angle) > QUAT_EPSILON)
	{
		const T sine = sin(cos_angle);
		const T inv_sine = 1./sine;
		RSin sinA(-cos_angle*inv_frames, cos_angle, inv_sine);
		RSin sinB(cos_angle*inv_frames, 0, inv_sine * bflip);

		for (int i=0; i<numFrames; i++) {
			//T amt = i*inv_frames;
			T a = sinA();
			T b = sinB();

			buffer[i].w = a*input.w + b*target.w;
			buffer[i].x = a*input.x + b*target.x;
			buffer[i].y = a*input.y + b*target.y;
			buffer[i].z = a*input.z + b*target.z;
			buffer[i].normalize();
		}
	} else {
		for (int i=0; i<numFrames; i++) {
			T a = i*inv_frames;
			T b = 1.-a;

			buffer[i].w = a*input.w + b*target.w;
			buffer[i].x = a*input.x + b*target.x;
			buffer[i].y = a*input.y + b*target.y;
			buffer[i].z = a*input.z + b*target.z;
			buffer[i].normalize();
		}
	}
}

template<typename T> 
Quat<T> Quat<T> :: getRotationTo(const Vec<3, T>& src, const Vec<3, T>& dst) {
	Quat<T> q;
	Vec<3, T> v0(src);
	Vec<3, T> v1(dst);
	
	T d = v0.dot(v1);
	if (d >= 1.) {
		// vectors are the same
		return q;
	}
	if (d < -0.999999999) {
		// vectors are nearly opposing
		// pick an axis to rotate around
		Vec<3, T> axis = cross(Vec<3, T>(0, 1, 0), src);
		// if colinear, pick another:
		if (axis.magSqr() < 0.00000000001) {
			axis = cross(Vec<3, T>(0, 0, 1), src);
		}
		axis.normalize();
		q.fromAxisAngle(180., axis);
	} else {
		T s = sqrt((d+1.)*2);
		T invs = 1./s;
		Vec<3, T> c = cross(v0, v1);
		q.x = c[0] * invs;
		q.y = c[1] * invs;
		q.z = c[2] * invs;
		q.w = s * 0.5;
	}
	return q.normalize();
}

/*!
	Get the quaternion from a given point and quaterion toward another point
*/
template<typename T>
void Quat<T> :: towardPoint(const Vec<3,T>& pos, const Quat<T>& q, const Vec<3,T>& v, float amt) {
	Vec<3,T> diff, axis;
	diff = v-pos;
	diff.normalize();
	
	if(amt < 0) {
		diff = diff*-1.;
	}

	Vec<3,T> zaxis;
	q.toVectorZ(zaxis);
	//axis = zaxis.cross(diff);
	cross(axis, zaxis, diff);
	//Vec<3,T>::cross(axis, zaxis, diff);
	axis.normalize();

	float axis_mag_sqr = axis.dot(axis);
	float along = zaxis.dot(diff); //Vec<3,T>::dot(zaxis, diff);

	if(axis_mag_sqr < 0.001 && along < 0) {
		//Vec<3,T>::cross(axis, zaxis, Vec<3,T>(0., 0., 1.));
		cross(axis, zaxis, Vec<3,T>(0, 0, 1));
		axis.normalize();

		if(axis_mag_sqr < 0.001) {
			//Vec<3,T>::cross(axis, zaxis, Vec<3,T>(0., 1., 0.));
			cross(axis, zaxis, Vec<3,T>(0, 1, 0));
			axis.normalize();
		}

		axis_mag_sqr = axis.dot(axis); //Vec<3,T>::dot(axis, axis);
	}

	if(along < 0.9995 && axis_mag_sqr > 0.001) {
		float theta = ABS(amt)*acos(along)*M_RAD2DEG;
//			printf("theta: %f  amt: %f\n", theta, amt);
		fromAxisAngle(theta, axis[0], axis[1], axis[2]);
	}
	else {
		setIdentity();
	}
}

// v1 and v2 must be normalized
// alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a)))
template<typename T>
Quat<T> Quat<T> :: rotor(const Vec<3,T>& v1, const Vec<3,T>& v2) {
	// get the normal to the plane (i.e. the unit bivector containing the v1 and v2)
	Vec<3,T> n;
	cross(n, v1, v2);
	n.normalize();	// normalize because the cross product can get slightly denormalized

	// calculate half the angle between v1 and v2
	T dotmag = v1.dot(v2);
	T theta = acos(dotmag)*0.5;

	// calculate the scaled actual bivector generaed by v1 and v2
	Vec<3,T> bivec = n*sin(theta);
	Quat<T> q(cos(theta), bivec[0], bivec[1], bivec[2]);

	return q;
}

} // namespace

#endif /* include guard */
