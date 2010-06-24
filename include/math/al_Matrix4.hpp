#ifndef INCLUDE_AL_MATRIX4_HPP
#define INCLUDE_AL_MATRIX4_HPP

#include "math/al_Quat.hpp"
#include "math/al_Vec.hpp"

namespace al {

template<class T> class Matrix4;

typedef Matrix4<double>	Matrix4d;	///< Double-precision quaternion
typedef Matrix4<float>	Matrix4f;	///< Single-precision quaternion

/// 4x4 Matrix (Homogenous Transform)
template<typename T=double>
class Matrix4 : public Mat<4, T> {	
public:
	typedef Mat<4, T> Base;

	Matrix4() { set(Identity()); }
	Matrix4(const T * src) { Base::set(src); }
	Matrix4(const Matrix4 & src) { set(src); }
	
	Matrix4& set(const Matrix4 & src) { Base::set(src.elems); return *this; }
	
	Quat<T>& toQuat() {
		Quat<T> q;
		q.fromMatrix(Base::elems);
		return q;
	}
	void fromQuat(Quat<T>& q) { q.toMatrix(Base::elems); }
		
	static const Matrix4 Identity() {
		const T m[] = {	1,	0,	0,	0, 
						0,	1,	0,	0, 
						0,	0,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	
	static const Matrix4 Translate(T x, T y, T z) {
		const T m[] = {	1,	0,	0,	x, 
						0,	1,	0,	y, 
						0,	0,	1,	z, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}	
	
	static const Matrix4 Scale(T x, T y, T z) {
		const T m[] = {	x,	0,	0,	0, 
						0,	y,	0,	0, 
						0,	0,	z,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	
	static const Matrix4 RotateX(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	1,	0,	0,	0, 
						0,	C,	-S,	0, 
						0,	S,	C,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 RotateY(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	C,	0,	S,	0, 
						0,	0,	0,	0, 
						-S,	0,	C,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4& RotateZ(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	C,	-S,	0,	0, 
						S,	C,	0,	0, 
						0,	0,	C,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}

//	static const Matrix4& Rotate(T a, T x, T y, T z) {
//		Vec<3, T> v(x, y, z);
//		Vec<3, T> u(v); u.normalize();
//		Mat<3, T> S(0, -u[2], u[1], u[2], 0, -u[0], -u[1], u[0], 0);
//		const T m[] = {	};
//		return Matrix4(m);
//	}
	
	static const Matrix4 ShearX(T y, T z) {
		const T m[] = {	1,	0,	0,	0, 
						y,	1,	0,	0, 
						z,	0,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 ShearY(T x, T z) {
		const T m[] = {	1,	x,	0,	0, 
						0,	1,	0,	0, 
						0,	z,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 ShearZ(T x, T y) {
		const T m[] = {	1,	0,	x,	0, 
						0,	1,	y,	0, 
						0,	0,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	
	static const Matrix4 Perspective(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T n2 = n*2;
		const T fn2 = f*n2;
		const T m[] = {	n2/W,	0,		W2/W,	0, 
						0,		n2/H,	H2/H,	0, 
						0,		0,		-D2/D,	-fn2/D,
						0,		0,		-1,		0 };
		return Matrix4(m);
	}
	
	static const Matrix4 UnPerspective(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T n2 = n*2;
		const T fn2 = f*n2;
		const T m[] = {	W/n2,	0,		0,		W2/n2, 
						0,		H/n2,	0,		H2/n2, 
						0,		0,		0,		-1, 
						0,		0,		-D/fn2,	D2/fn2 };
		return Matrix4(m);
	}
	
	static const Matrix4 Ortho(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T m[] = {	2/W,	0,		0,		-W2/W, 
						0,		2/H,	0,		-H2/H, 
						0,		0,		-2/D,	-D2/D, 
						0,		0,		0,		1 };
		return Matrix4(m);
	}
	
	static const Matrix4 UnOrtho(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T m[] = {	W/2,	0,		0,		W2/2, 
						0,		H/2,	0,		H2/2, 
						0,		0,		D/-2,	D2/2, 
						0,		0,		0,		1 };
		return Matrix4(m);
	}
		
};






/// Implementation


} // namespace

#endif /* include guard */
