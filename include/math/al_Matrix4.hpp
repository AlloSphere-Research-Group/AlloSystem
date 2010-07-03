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

	Matrix4()
	: Base(
		1, 0, 0, 0, 
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	)
	{}
	
	Matrix4(
		const T& v11, const T& v21, const T& v31, const T& v41,
		const T& v12, const T& v22, const T& v32, const T& v42,
		const T& v13, const T& v23, const T& v33, const T& v43,
		const T& v14, const T& v24, const T& v34, const T& v44
	)
	:	Base(
			v11, v12, v13, v14,
			v21, v22, v23, v24,
			v31, v32, v33, v34,
			v41, v42, v43, v44
		)
	{}
	
	Matrix4(const T * src)
	:	Base(src)
	{}
	
	Matrix4(const Base & src)
	:	Base(src)
	{}
	
	Matrix4& set(const Base & src) { Base::set(src.elems); return *this; }
	
	Quat<T>& toQuat() {
		Quat<T> q;
		q.fromMatrix(Base::elems);
		return q;
	}
	void fromQuat(Quat<T>& q) { q.toMatrix(Base::elems); }
		
	static const Matrix4 Identity() {
		Matrix4 m(
			1,	0,	0,	0, 
			0,	1,	0,	0, 
			0,	0,	1,	0, 
			0,	0,	0,	1
		);
		return m;
	}
	
	static const Matrix4 Translate(T x, T y, T z) {
		Matrix4 m(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			x, y, z, 1
		);
		return m;
	}	
	
	static const Matrix4 Scale(T x, T y, T z) {
		Matrix4 m;/*(
			x,	0,	0,	0,
			0,	y,	0,	0, 
			0,	0,	z,	0, 
			0,	0,	0,	1
		);*/
		
		return m;
	}
	
	static const Matrix4 RotateYZ(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	1,	0,	0,	0, 
						0,	C,	-S,	0, 
						0,	S,	C,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 RotateZX(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	C,	0,	S,	0, 
						0,	1,	0,	0, 
						-S,	0,	C,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 RotateXY(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		const T m[] = {	C,	-S,	0,	0, 
						S,	C,	0,	0, 
						0,	0,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}

	static const Matrix4 rotate(float angle, const Vec<3, T> &v) {
		Vec<3, T> axis(v);
		axis.normalize();
		
		float c = cos(angle*QUAT_DEG2RAD);
		float s = sin(angle*QUAT_DEG2RAD);
			
		Matrix4 m(
			axis[0]*axis[0]*(1-c)+c,
			axis[1]*axis[0]*(1-c)+axis[2]*s,
			axis[0]*axis[2]*(1-c)-axis[1]*s,
			0,
			
			axis[0]*axis[1]*(1-c)-axis[2]*s,
			axis[1]*axis[1]*(1-c)+c,
			axis[1]*axis[2]*(1-c)+axis[0]*s,
			0,

			axis[0]*axis[2]*(1-c)+axis[1]*s,
			axis[1]*axis[2]*(1-c)-axis[0]*s,
			axis[2]*axis[2]*(1-c)+c,
			0,
			
			0, 0, 0, 1
		);
		
		return m;
	}
	
	static const Matrix4 ShearYZ(T y, T z) {
		const T m[] = {	1,	0,	0,	0, 
						y,	1,	0,	0, 
						z,	0,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 ShearZX(T z, T x) {
		const T m[] = {	1,	x,	0,	0, 
						0,	1,	0,	0, 
						0,	z,	1,	0, 
						0,	0,	0,	1 };
		return Matrix4(m);
	}
	static const Matrix4 ShearXY(T x, T y) {
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
