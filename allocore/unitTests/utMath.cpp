#include <math.h>
#include "utAllocore.h"

template <class T>
inline bool eq(T x, T y, T eps=0.000001){
	return abs(x-y) < eps;
}

template <class T>
inline bool eq(const T* x, const T* y, int n, T eps=0.0000001){
	for(int i=0; i<n; ++i){
		if(!eq(x[i], y[i], eps)) return false;
	}
	return true;
}

template <class T>
inline bool eq(const Quat<T>& a, const Quat<T>& b, T eps=0.000001){
	return eq(&a[0], &b[0], 4, eps);
}

template <int N, class T>
inline bool eq(const Vec<N,T>& a, const Vec<N,T>& b, T eps=0.000001){
	return eq(&a[0], &b[0], N, eps);
}

template <int N, class T>
inline bool eq(const Mat<N,T>& a, const Mat<N,T>& b, T eps=0.000001){
	return eq(&a[0], &b[0], N*N, eps);
}


int utMath(){

	// Vec
	{
		// Should be able to hold objects with constructors
		{ Vec<1, Vec<1, int> > t; }
		{ Vec<4, Complex<float> > t; }
		{ Vec<5, char> t; }
		{ Vec<0,int> t; }

		const int N = 4;

		Vec<N,double> a, b, c;	assert(a.size() == N);

		a[0] = 0;				assert(a[0] == 0);
								assert(a.elems()[0] == 0);

		a.x = 1;				assert(a[0] == 1);
		a.y = 2;				assert(a[1] == 2);
		a.z = 3;				assert(a[2] == 3);
		a.w = 4;				assert(a[3] == 4);

		a = 1;					assert(a == 1);
		b = a;					assert(b == 1);
								assert(b == a);
		b = 2;					assert(b != a);

		{
		a = 1;
		b = 0;
		double * p = a.elems();	assert(p[0] == a[0]);
		b.set(p);				assert(a == b);

		char c1[] = {4,4,4,4};
		a.set(c1);				assert(a == 4);

		char c2[] = {1,0,1,0,1,0,1,0};
		a.set(c2,2);			assert(a == 1);

		a = 0;
		a.set(Vec<N-1,int>(1,2,3), 4);
								assert(a[0] == 1 && a[1] == 2 && a[2] == 3 && a[3] == 4);
		}

		a = 3;
		b = 3;
		a -= b;					assert(a == 0);
		a += b;					assert(a == 3);
		a -= 3;					assert(a == 0);
		a += 3;					assert(a == 3);

		a *= b;					assert(a == 9);
		a /= b;					assert(a == 3);
		a *= 3;					assert(a == 9);
		a /= 3;					assert(a == 3);

		a = b + b;				assert(a == 6);
		a = b - b;				assert(a == 0);
		a = b * b;				assert(a == 9);
		a = b / b;				assert(a == 1);

		a = 2. + a;				assert(a == 3);
		a = 6. - a;				assert(a == 3);
		a = 2. * a;				assert(a == 6);
//		a = 1. / a;				assert(a == 1./6);

		a = +1;
		b = -1;					assert(a == -b);

		a = -1;
		b = +1;
		assert(a.dot(b) ==-N);
		assert(a.dot(a) == N);
		assert(a.sum() == -N);
		assert(a.sumAbs() == N);
		assert(a.mag() == sqrt(N));
		assert(b.mag() == sqrt(N));
		assert(a.mag() == abs(a));
		assert(a.magSqr() == N);
		assert(b.magSqr() == N);
		assert(a.norm1() == N);
		assert(a.norm2() == sqrt(N));

		(a = 1).negate();		assert(a == -1);
		(a = 1).normalize();	assert(a == 1./sqrt(N));
		assert(a == (b = 10).normalized());

		// constructors
		{ decltype(a) t(a);			assert(t == a); }
		{ Vec<3,float> t(1);		assert(t[0] == 1 && t[1] == 1 && t[2] == 1); }
		{ Vec<3,float> t(1,2);		assert(t[0] == 1 && t[1] == 2 && t[2] == 0); }
		{ Vec<3,float> t(1,2,3);	assert(t[0] == 1 && t[1] == 2 && t[2] == 3); }
		{ Vec<3,float> t({1,2,3});	assert(t[0] == 1 && t[1] == 2 && t[2] == 3); }
		{ Vec<3,float> t(Vec<2,float>(1,2), 3); assert(t[0] == 1 && t[1] == 2 && t[2] == 3); }
		{ Vec<3,float> t(Vec<2,float>(1,2)); assert(t[0] == 1 && t[1] == 2 && t[2] == 0); }
		{	float s[] = {1,2,3};
			Vec<3,float> t(s);		assert(t[0] == 1 && t[1] == 2 && t[2] == 3);
		}
		{	float s[] = {1,10,2,20,3,30};
			Vec<3,float> t(s,2);	assert(t[0] == 1 && t[1] == 2 && t[2] == 3);
		}

		// access
		for(int i=0; i<a.size(); ++i) a[i]=i;
		assert(a.get(0,1) == Vec2d(0,1));
		assert(a.get(2,2) == Vec2d(2,2));
		assert(a.get(2,1,0) == Vec3d(2,1,0));

		{
		for(int i=0; i<a.size(); ++i) a[i]=i;

		Vec<2, double> t;
		t = a.sub<2>();			assert(t[0] == 0 && t[1] == 1);
		t = a.sub<2>(2);		assert(t[0] == 2 && t[1] == 3);
			// verify in-place operations
		a.sub<2>() += 10;		assert(a[0] == 10 && a[1] == 11);
		}

		// Vec-Vec ops
		b = a = 1;
		assert(concat(a,b) == 1);

		{
		a = 0;
		Vec<N+1, double> t = concat(a, Vec<1,char>(1));
		assert(t.size() == a.size()+1);
		}

		// geometry and other math ops
		assert(eq(angle(Vec3d(1,0,0), Vec3d(1, 0, 0)), 0.));
		assert(eq(angle(Vec3d(1,0,0), Vec3d(0, 1, 0)), M_PI_2));
		assert(eq(angle(Vec3d(1,0,0), Vec3d(0,-1, 0)), M_PI_2));

		{
		Vec3d r;
		centroid3(r, Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,0,1));
		assert(eq(r, Vec3d(1/3.)));

		normal(r, Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(-1,0,0));
		assert(eq(r, Vec3d(0,0,1)));
		}

		a = 0;
		b = 1;
		assert(min(a,b) == 0);
		assert(max(a,b) == 1);

	}


	// Vec3
	{
		Vec3d a, b, c;

		a.set(1,0,0);
		b.set(0,1,0);
		c.set(0,0,1);
			assert(c == cross(a,b));
			assert(c == a.cross(b));

		a = b;
			assert(a == b);
	}


	// Mat
	{
		/*#define CHECK(mat, a,b,c, d,e,f, g,h,i){\
			const auto& m = mat;\
			assert(eq(m(0,0),a)); assert(eq(m(0,1),b)); assert(eq(m(0,2),c));\
			assert(eq(m(1,0),d)); assert(eq(m(1,1),e)); assert(eq(m(1,2),f));\
			assert(eq(m(2,0),g)); assert(eq(m(2,1),h)); assert(eq(m(2,2),i));\
		}*/
		#define CHECK(mat, a,b,c, d,e,f, g,h,i)\
			assert(eq(mat, Mat3d(a,b,c, d,e,f, g,h,i)))

		// factory functions
		CHECK(Mat3d::identity(),				1,0,0, 0,1,0, 0,0,1);
		CHECK(Mat3d::scaling(2),				2,0,0, 0,2,0, 0,0,1);
		CHECK(Mat3d::scaling(2,3),				2,0,0, 0,3,0, 0,0,1);
		CHECK(Mat3d::scaling(Vec2d(2,3)),		2,0,0, 0,3,0, 0,0,1);
		CHECK(Mat3d::translation(2,3),			1,0,2, 0,1,3, 0,0,1);
		CHECK(Mat3d::translation(Vec2d(2,3)),	1,0,2, 0,1,3, 0,0,1);
		CHECK(Mat3d::rotation(M_PI/2, 0,1),		0,-1,0,1,0,0, 0,0,1);

		Mat3d a;//, b;

		a.setIdentity();	CHECK(a, 1,0,0, 0,1,0, 0,0,1);

		assert(a.trace() == 3);

		a += 2;		CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a -= 1;		CHECK(a, 2,1,1, 1,2,1, 1,1,2);
		a *= 2;		CHECK(a, 4,2,2, 2,4,2, 2,2,4);
		a /= 2;		CHECK(a, 2,1,1, 1,2,1, 1,1,2);

		a.setIdentity();
		a = a+2;	CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a = a-1;	CHECK(a, 2,1,1, 1,2,1, 1,1,2);
		a = a*2;	CHECK(a, 4,2,2, 2,4,2, 2,2,4);
		a = a/2;	CHECK(a, 2,1,1, 1,2,1, 1,1,2);

		a.setIdentity();
		a = 2.+a;	CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a = 4.-a;	CHECK(a, 1,2,2, 2,1,2, 2,2,1);
		a = 2.*a;	CHECK(a, 2,4,4, 4,2,4, 4,4,2);

		a.set(	1,2,3,
				4,5,6,
				7,8,9
		);

		assert(a.col(0) == Vec3d(1,4,7));
		assert(a.col(1) == Vec3d(2,5,8));
		assert(a.col(2) == Vec3d(3,6,9));
		assert(a.row(0) == Vec3d(1,2,3));
		assert(a.row(1) == Vec3d(4,5,6));
		assert(a.row(2) == Vec3d(7,8,9));

		a.transpose();

		assert(a.col(0) == Vec3d(1,2,3));
		assert(a.col(1) == Vec3d(4,5,6));
		assert(a.col(2) == Vec3d(7,8,9));
		assert(a.row(0) == Vec3d(1,4,7));
		assert(a.row(1) == Vec3d(2,5,8));
		assert(a.row(2) == Vec3d(3,6,9));

		// test special operations
		{
			Mat<2,double> m(
				2,4,
				0,3
			);

			assert(eq(determinant(m), 6.));

			Mat<2,double> inv = m;
			assert(invert(inv));
			assert(eq(m*inv, Mat<2,double>::identity()));
		}

		{
			Mat<3,double> m(
				2,5,7,
				0,3,6,
				0,0,4
			);

			assert(eq(determinant(m), 24.));

			Mat<3,double> inv = m;
			assert(invert(inv));
			assert(eq(m*inv, Mat<3,double>::identity()));
		}

		#undef CHECK
	}


	{	Complexd c(0,0);
		#define T(x, y) assert(x == y);
		T(c, Complexd(0,0))
		c.fromPolar(1, 0.2);	T(c, Polard(0.2))
		c.fromPolar(2.3);		T(c, Polard(2.3))
		assert(c != Complexd(0,0));
		T(c.conj(), Complexd(c.r, -c.i))
		#undef T

//		#define T(x, y) assert(almostEqual(x,y,2));
//		c.normalize();			T(c.norm(), 1)
//		double p=0.1; c(1,0); c *= Polard(1, p); T(c.arg(), p)
//
//		c.fromPolar(4,0.2);
//		T(c.sqrt().norm(), 2)
//		T(c.sqrt().arg(), 0.1)
//		#undef T
	}

	// Quat
	{
		struct printQuat{
			void operator()(const Quatd& v){ printf("%g %g %g %g\n", v[0], v[1], v[2], v[3]); }
		};

		// Test factory functions
		assert(Quatd::identity() == Quatd(1,0,0,0));


		// Test basic mathematical operations
		Quatd q(0,0,0,0);

		assert(q == Quatd(0,0,0,0));

		q.setIdentity();

		assert(q == Quatd(1,0,0,0));
		assert(q != Quatd(1,0,0,1));

		q.set(1,2,4,10);

		assert(-q == Quatd(-1,-2,-4,-10));
		assert(q.conj()	== Quatd(q.w, -q.x, -q.y, -q.z));
		assert(q.dot(q) == 121);
		assert(q.mag() == 11);
		assert(q.magSqr() == 121);
		assert(eq(q.sgn(), Quatd(1./11, 2./11, 4./11, 10./11)));


		// Test rotation of vectors by quaternion
		q.fromAxisAngle(M_2PI/4, 1,0,0);
		assert(eq(q, Quatd(sqrt(2)/2, sqrt(2)/2,0,0)));
		{
			Vec3d v(0,1,0);
			v = q.rotate(v);
			//printf("%g %g %g\n", v[0], v[1], v[2]);
			assert(eq(v, Vec3d(0,0,1)));
		}

		q.fromAxisAngle(M_2PI/4, 0,1,0);
		assert(eq(q, Quatd(sqrt(2)/2, 0,sqrt(2)/2,0)));
		{
			Vec3d v(0,0,1);
			v = q.rotate(v);
			//printf("%g %g %g\n", v[0], v[1], v[2]);
			assert(eq(v, Vec3d(1,0,0)));
		}

		q.fromAxisAngle(M_2PI/4, 0,0,1);
		{
			Vec3d v(1,0,0);
			v = q.rotate(v);
			//printf("%g %g %g\n", v[0], v[1], v[2]);
			assert(eq(v, Vec3d(0,1,0)));
		}

		// Test fromAxis* consistency
		assert(q.fromAxisAngle(M_2PI/8, 1,0,0) == Quatd().fromAxisX(M_2PI/8));
		assert(q.fromAxisAngle(M_2PI/8, 0,1,0) == Quatd().fromAxisY(M_2PI/8));
		assert(q.fromAxisAngle(M_2PI/8, 0,0,1) == Quatd().fromAxisZ(M_2PI/8));

		// Test AxisAngle<->Quat conversion
		{
			q.fromEuler(M_2PI/7, M_2PI/8, M_2PI/9);		// set to something non-trival...
			double angle, ax,ay,az;
			q.toAxisAngle(angle, ax,ay,az);
			Quatd b = q.fromAxisAngle(angle, ax,ay,az);
			assert(q == b || q == b.conj());
		}

		// Test consistency between conversions from Euler angles and axes
		assert(
			eq(
			q.fromEuler(M_2PI/8,M_2PI/8,M_2PI/8),
			(Quatd().fromAxisY(M_2PI/8) * Quatd().fromAxisX(M_2PI/8)) * Quatd().fromAxisZ(M_2PI/8)
			)
		);

		// Test roundtrip Euler/quat conversion
		{
			float eps = 0.00001;
			int N = 16;
			for(int k=0; k<N; ++k){
			for(int j=0; j<N; ++j){
			for(int i=0; i<N; ++i){
				float kludge = 0.1;
				float az = (float(k)/N-0.5) * 2*M_PI + kludge;
				float el = (float(j)/N-0.5) * 2*M_PI + kludge;
				float ba = (float(i)/N-0.5) * 2*M_PI + kludge;

				Quatf a,b;
				a.fromEuler(az, el, ba);
				a.toEuler(az, el, ba);
				b.fromEuler(az, el, ba);
				//a.print(); b.print(); printf("\n");

				assert(eq(a, b, eps) || eq(a, -b, eps));
			}}}
		}

		// Test Quat to matrix
		{
			Quatd q;
			q.set(1,0,0,0);
			Mat4d m;
			q.toMatrix(&m[0]);
			assert(eq(m, Mat4d::identity()));
			
			
			q.fromAxisAngle(M_2PI/4, 0,0,1);
			q.toMatrix(&m[0]);
			// For a right-handed coordinate system
			assert(eq(m, Mat4d(0,-1,0,0, 1,0,0,0, 0,0,1,0, 0,0,0,1)));
		}

		// Test Quat to component coordinate frame
		{
			Quatd q;

			Vec3d vx, vy, vz;

			q.fromAxisAngle(M_2PI/4, 1,0,0);
			q.toVectorX(vx);
			q.toVectorY(vy);
			q.toVectorZ(vz);
			assert(eq(vx, Vec3d(1, 0,0)));
			assert(eq(vy, Vec3d(0, 0,1)));
			assert(eq(vz, Vec3d(0,-1,0)));

			q.fromAxisAngle(M_2PI/4, 0,1,0);
			q.toVectorX(vx);
			q.toVectorY(vy);
			q.toVectorZ(vz);
			assert(eq(vx, Vec3d(0,0,-1)));
			assert(eq(vy, Vec3d(0,1, 0)));
			assert(eq(vz, Vec3d(1,0, 0)));

			q.fromAxisAngle(M_2PI/4, 0,0,1);
			q.toVectorX(vx);
			q.toVectorY(vy);
			q.toVectorZ(vz);
			assert(eq(vx, Vec3d(0,1,0)));
			assert(eq(vy, Vec3d(-1,0,0)));
			assert(eq(vz, Vec3d(0,0,1)));
		}

		// Test roundtrip matrix/quat conversions
		{
			double mat4[16];
			Quatd b;
			q.fromEuler(1.,2.,3.); // a non-trival quat
			q.toMatrix(mat4);
			b = q.fromMatrix(mat4);
			assert( eq(q,b) || eq(q,b.conj()) );

			q.toMatrixTransposed(mat4);
			b = q.fromMatrixTransposed(mat4);
			assert( eq(q,b) || eq(q,b.conj()) );
		}

//		int smps = 100;
//		Quatd q1 = Quatd::fromAxisAngle(10, .707, .707, 0);
//		Quatd q2 = Quatd::fromAxisAngle(60, .707, 0, .707);
//		Quatd buf[smps];
//		Quatd::slerp_buffer(q1, q2, buf, smps);
//		for (int i=0; i<smps; i++) {
//			double t, x, y, z;
//			buf[i].toAxisAngle(&t, &x, &y, &z);
//			//printf("%f %f %f %f\n", t, x, y, z);
//		}
	}



	// Simple Functions
	{
	const double pinf = INFINITY;		// + infinity
	const double ninf =-INFINITY;		// - infinity

	#define T(x) assert(al::abs(x) == (x<0?-x:x));
	T(0.) T(1.) T(-1.)
	T(0) T(1) T(-1)
	#undef T

	#define T(x,y, r) assert(al::atLeast(x,y) == r);
	T(0.,1., 1.) T(+0.1,1., 1.) T(-0.1,1., -1.)
	#undef T

	#define T(x,y) assert(al::abs(al::atan2Fast(x,y) - std::atan2(x,y)) < 1e-5);
	T(1.,0.) T(1.,1.) T(0.,1.) T(-1.,1.) T(-1.,0.) T(-1.,-1.) T(0.,-1.) T(1.,-1.)
	#undef T

	#define T(x, y) assert(al::ceil(x) == y);
	T(0., 0.)	T( 1., 1.) T( 1.2, 2.) T( 1.8, 2.) T( 1000.1, 1001.)
				T(-1.,-1.) T(-1.2,-1.) T(-1.8,-1.) T(-1000.1,-1000.)
	#undef T

	#define T(x, y) assert(al::ceilEven(x) == y);
	T(0, 0) T(1, 2) T(2, 2) T(3, 4) T(1001, 1002)
	#undef T

	#define T(x, y) assert(al::ceilPow2(x) == y);
	T(0, 0) T(1, 1) T(2, 2) T(3, 4)
	T(500, 512) T(999, 1024)
	#undef T

	#define T(x, y) assert(al::clip(x) == y);
	T(0., 0.) T(0.5, 0.5) T(1., 1.) T(1.2, 1.) T(-0.5, 0.) T(pinf, 1.) T(ninf, 0.)
	#undef T

	#define T(x, y) assert(al::clipS(x) == y);
	T(0., 0.) T(0.5, 0.5) T(1., 1.) T(1.2, 1.) T(-0.5, -0.5) T(-1., -1) T(-1.2, -1.)
	#undef T

	#define T(x,r) assert(al::even(x) == r);
	T(0,true) T(1,false) T(-2,true)
	#undef T

	#define T(x, y) assert(al::factorial(x) == y);
	T(0, 1) T(1, 1) T(2, 2*1) T(3, 3*2*1) T(4, 4*3*2*1)
	T(5, 5*4*3*2*1) T(6, 6*5*4*3*2*1) T(7, 7*6*5*4*3*2*1) T(8, 8*7*6*5*4*3*2*1)
	T(9, 9*8*7*6*5*4*3*2*1) T(10, 10*9*8*7*6*5*4*3*2*1) T(11, 11*10*9*8*7*6*5*4*3*2*1)
	T(12, 12*11*10*9*8*7*6*5*4*3*2*1)
	#undef T

	for(int i=0; i<=12; ++i){
		assert(
			al::aeq(al::factorialSqrt(i), sqrt(al::factorial(i)))
		);
	}

	#define T(x, y) assert(al::floor(x) == y);
	T(0., 0.)	T( 1., 1.) T( 1.2, 1.) T( 1.8, 1.) T( 1000.1, 1000.)
				T(-1.,-1.) T(-1.2,-2.) T(-1.8,-2.) T(-1000.1,-1001.)
	#undef T

	#define T(x, y) assert(al::floorPow2(x) == y);
	T(0, 1) T(1, 1) T(2, 2) T(3, 2)
	T(513, 512) T(1090, 1024)
	#undef T

	#define T(x, y) assert(eq(al::fold(x), y));
	T(0., 0.) T(0.5, 0.5) T(1., 1.) T(1.2, 0.8) T(-0.2, 0.2)
	T(2.2, 0.2) T(3.2, 0.8) T(4.2, 0.2) T(5.2, 0.8)
	#undef T

	#define T(x,y,r) assert(al::gcd(x,y) == r);
	T(7,7,7) T(7,4,1) T(8,4,4)
	#undef T

	#define T(x,y,r) assert(al::lcm(x,y)==r);
	T(7,3,21) T(8,4,8) T(3,1,3)
	#undef T

	#define T(x,y,r) assert(al::lessAbs(x,y)==r);
	T(0.1,1., true) T(-0.1,1., true) T(1.,1., false) T(-1.,1., false)
	#undef T

	#define T(x) assert(al::log2(1<<x) == x);
	T(0) T(1) T(2) T(3) T(4) T(29) T(30) T(31)
	#undef T

	#define T(x,y,r) assert(al::max(x,y)==r);
	T(0,0,0) T(0,1,1) T(1,0,1) T(-1,1,1)
	#undef T

	#define T(x,y,z,r) assert(al::max(x,y,z)==r);
	T(0,0,0, 0) T(0,1,2, 2) T(1,2,0, 2) T(2,1,0, 2)
	#undef T

	assert(al::mean(-1., 1.) == 0.);

	#define T(x,y,r) assert(al::min(x,y)==r);
	T(0,0,0) T(0,1,0) T(1,0,0) T(-1,1,-1)
	#undef T

	#define T(x,r) assert(al::odd(x) == r);
	T(0,false) T(1,true) T(-2,false)
	#undef T

	#define T(x) assert(al::pow2(x) == x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow2S(x) == x*al::abs(x));
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow3(x) == x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow3Abs(x) == al::abs(x*x*x));
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow4(x) == x*x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow5(x) == x*x*x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow6(x) == x*x*x*x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow8(x) == x*x*x*x*x*x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(al::pow16(x) == x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x);
	T(0) T(1) T(2) T(3) T(-1) T(-2) T(-3)
	#undef T

	#define T(x) assert(eq(al::pow64(x), x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x));
	T(0.) T(1.) T(1.01) T(1.02) T(-1.) T(-1.01) T(-1.02)
	#undef T

//	#define T(x,r) assert(al::powerOf2(x) == r);
//	T(0, false) T(1, true) T(2, true) T(3, false) T(4, true)
//	#undef T

	#define T(x,y,r) assert(al::remainder(x,y) == r);
	T(7,7,0) T(7,1,0) T(7,4,3) T(7,3,1) T(14,3,2)
	#undef T

	#define T(x,y) assert(al::round(x) == y);
	T(0.f, 0.f) T(0.2f, 0.f) T(0.8f, 1.f) T(-0.2f, 0.f) T(-0.8f,-1.f) T(0.5f, 1.f) T(-0.5f, -1.f)
	T(0.0, 0.0) T(0.20, 0.0) T(0.80, 1.0) T(-0.20, 0.0) T(-0.80,-1.0) T(0.50, 1.0) T(-0.50, -1.0)
	#undef T

	#define T(x,y,r) assert(al::round(x,y) == r);
	T(0.0,0.1, 0.0) T(0.1,0.1, 0.1) T(0.15,0.1, 0.1) T(-0.15,0.1, -0.1)
	#undef T

	#define T(x,y,r) assert(al::roundAway(x,y) == r);
	T(0.0,0.1, 0.0) T(0.1,0.1, 0.1) T(0.15,0.1, 0.2) T(-0.15,0.1, -0.2)
	#undef T

	#define T(x,r) assert(al::sgn(x) == r);
	T(-0.1, -1.) T(0.1, 1.) T(0., 0.)
	#undef T

	#define T(x1,y1,x2,y2, r) assert(al::slope(x1,y1,x2,y2) == r);
	T(3.,3.,4.,4., 1.) T(3.,-3.,4.,-4., -1.)
	#undef T

	{	double x=1, y=0;
		sort(x,y); assert(x==0 && y==1);
		sort(x,y); assert(x==0 && y==1);
	}

	#define T(x,y) assert(al::sumOfSquares(x) == y);
	T(1., 1.) T(2., 1*1+2*2) T(3., 1*1+2*2+3*3) T(4., 1*1+2*2+3*3+4*4) T(5., 1*1+2*2+3*3+4*4+5*5)
	#undef T

	#define T(x,r) assert(al::trailingZeroes(x) == r);
	T(0, 0) T(1, 0) T(2, 1) T(3, 0) T(4, 2) T(8, 3) T(9, 0)
	#undef T

	#define T(x,y) assert(al::trunc(x) == y);
	T(0.f, 0.f) T(0.2f, 0.f) T(0.8f, 0.f) T(-0.2f, 0.f) T(-0.8f, 0.f) T(0.5f, 0.f) T(-0.5f, 0.f)
	T(0.0, 0.0) T(0.20, 0.0) T(0.80, 0.0) T(-0.20, 0.0) T(-0.80, 0.0) T(0.50, 0.0) T(-0.50, 0.0)
	#undef T

	#define T(x,l,h,r) assert(al::within(x,l,h)==r);
	T(0,0,1, true) T(1,0,1, true)
	#undef T

//	printf("%.20g\n", wrap<double>(-32.0, 32.0, 0.));  // should be 0.0
//	printf("%.20g\n", wrap<double>(-64.0, 32.0, 0.));  // should be 0.0
//	printf("%.20g\n", wrap<double>(-1e-16, 32., 0.));  // should be 31.999999999999996447

	#define T(x, y) assert(eq(al::wrap(x, 1., -1.), y));
	T(0., 0.)	T( 0.5, 0.5) T( 1.,-1.) T( 1.2,-0.8) T( 2.2, 0.2)
				T(-0.5,-0.5) T(-1.,-1.) T(-1.2, 0.8) T(-2.2,-0.2)
	#undef T
	}

	#define T(x, y) assert(eq(al::wrapPhase(x), y));
	T(0., 0.)	T( 1., 1.) T( M_PI,-M_PI) T( M_PI+1, -M_PI+1) T( 7*M_PI+1, -M_PI+1)
				T(-1.,-1.) T(-M_PI,-M_PI) T(-M_PI-1,  M_PI-1) T(-7*M_PI+1, -M_PI+1)
	#undef T

	// Special Functions
	{
		struct F{

			// Pl,-m(x) = (-1)^m (l-m)! / (l+m)! Pl,m(x)
			static double testLegendreP(int l, int m, double x){
				switch(l){
				case 0:			return 1;
				case 1:
					switch(m){
					case -1:	return -1./(2) * testLegendreP(l,-m,x);
					case  0:	return x;
					case  1:	return -sqrt(1 - x*x);
					}
					break;
				case 2:
					switch(m){
					case -2:	return +1./(4*3*2*1) * testLegendreP(l,-m,x);
					case -1:	return -1./(  3*2  ) * testLegendreP(l,-m,x);
					case  0:	return 0.5 * (3*x*x - 1);
					case  1:	return -3 * x * sqrt(1 - x*x);
					case  2:	return 3 * (1 - x*x);
					}
					break;
				case 3:
					switch(m){
					case -3:	return -1. / (6*5*4*3*2*1) * testLegendreP(l,-m,x);
					case -2:	return +1. / (  5*4*3*2  ) * testLegendreP(l,-m,x);
					case -1:	return -1. / (    4*3    ) * testLegendreP(l,-m,x);
					case  0:	return 0.5 * x * (5*x*x - 3);
					case  1:	return 1.5 * (1 - 5*x*x) * sqrt(1-x*x);
					case  2:	return 15 * x * (1 - x*x);
					case  3:	return -15 * al::pow3(sqrt(1-x*x));
					}
					break;
				}
				return 0;	// undefined
			}

			static double testLaguerre(int n, int k, double x){
				switch(n){
				case 0:	return 1;
				case 1:	return -x + k + 1;
				case 2: return (1./2)*(x*x - 2*(k+2)*x + (k+2)*(k+1));
				case 3: return (1./6)*(-x*x*x + 3*(k+3)*x*x - 3*(k+2)*(k+3)*x + (k+1)*(k+2)*(k+3));
				default: return 0;
				}
			}
		};


		const int M = 2000;	// granularity of domain

		// test associated legendre
		for(int l=0; l<=3; ++l){
		for(int m=0; m<=l; ++m){
		for(int i=0; i<M; ++i){
			double theta = double(i)*M_PI / M;
			double a = al::legendreP(l, m, theta);
			double b = F::testLegendreP(l, m, cos(theta));
	//		if(!al::aeq(a, b, 1<<16)){
			if(!(al::abs(a - b)<1e-10)){
				printf("\nP(%d, %d, %g) = %.16g (actual = %.16g)\n", l,m, cos(theta), a,b);
				assert(false);
			}
		}}}

		// test laguerre
		for(int n=0; n<=3; ++n){
		for(int i=0; i<M; ++i){
			double x =double(i)/M * 4;
			double a = al::laguerreL(n,1, x);
			double b = F::testLaguerre(n,1, x);

			if(!(al::abs(a - b)<1e-10)){
				printf("\nL(%d, %g) = %.16g (actual = %.16g)\n", n, x, a,b);
				assert(false);
			}
		}}

		// TODO: spherical harmonics
//		for(int l=0; l<=SphericalHarmonic::L_MAX; ++l){
//		for(int m=-l; m<=l; ++m){
//	//		double c = SphericalHarmonic::coef(l,m);
//	//		double t = computeCoef(l,m);
//	//		assert(c == t);
//		}}
//
//		for(int j=0; j<M; ++j){	double ph = double(j)/M * M_PI;
//		for(int i=0; i<M; ++i){ double th = double(i)/M * M_2PI;
//
//		}}
	}


	// Interval
	{
		Interval<double> i(0,1);

		assert(i.min()==0 && i.max()==1);

		i.min(2);
		assert(i.min()==1 && i.max()==2);

		i.max(0);
		assert(i.min()==0 && i.max()==1);

		i.endpoints(-1,1);
		assert(i.min()==-1 && i.max()==1);

		assert(i.center()	==0);
		assert(i.diameter()	==2);
		assert(i.radius()	==1);

		assert(i.proper());

		i.endpoints(0,0);
		assert(i.degenerate());

		i.centerDiameter(1, 4);
		assert(i.center()	==1);
		assert(i.diameter()	==4);
		assert(i.min()==-1 && i.max()==3);

		i.center(2);
		assert(i.min()==0 && i.max()==4);

		i.diameter(6);
		assert(i.min()==-1 && i.max()==5);

		i.endpoints(-1, 1);
		assert(i.toUnit(0) == 0.5);

		assert(Interval<int>(0,1) == Interval<int>(0,1));
		assert(Interval<int>(0,2) != Interval<int>(0,1));
		assert((Interval<int>(0,2) += Interval<int>(-1,2)) == Interval<int>(-1,4));
		assert((Interval<int>(0,2) -= Interval<int>(-1,2)) == Interval<int>(-2,3));
	}


	// Random
	{
		using namespace al::rnd;

		// Ensure uniqueness of sequences
		assert(seed() != seed());

		{	LinCon a,b;
			assert(a() != b()); // sequences are unique
			assert(a() != a());	// successive values are unique
		}

		{	MulLinCon a,b;
			assert(a() != b()); // sequences are unique
			assert(a() != a()); // successive values are unique
		}

		{	Tausworthe a,b;
			assert(a() != b()); // sequences are unique
			assert(a() != a()); // successive values are unique
		}


		Random<> r;
		int N = 1000000;
		for(int i=0; i<N; ++i){ float v=r.uniform(); assert(  0 <= v && v < 1); }
		for(int i=0; i<N; ++i){ int v=r.uniform(20,  0); assert(  0 <= v && v < 20); }
		for(int i=0; i<N; ++i){ int v=r.uniform(20, 10); assert( 10 <= v && v < 20); }
		for(int i=0; i<N; ++i){ int v=r.uniform(20,-10); assert(-10 <= v && v < 20); }

		for(int i=0; i<N; ++i){ float v=r.uniformS(); assert(  -1 <= v && v <  1); }
		for(int i=0; i<N; ++i){ int v=r.uniformS(20); assert( -20 <= v && v < 20); }

		//for(int i=0; i<32; ++i) printf("% g ", r.uniformS());
		//for(int i=0; i<32; ++i) printf("%d ", r.prob(0.1));
		//for(int i=0; i<128; ++i) printf("% g\n", r.gaussian());

		int arr[] = {0,1,2,3,4,5,6,7};
		r.shuffle(arr, 8);
		//for(int i=0; i<8; ++i) printf("%d\n", arr[i]);
		//printf("\n");

		// Test uniformity of random sequence
		{
			Random<> r;
			const int N=64;
			const int M=1000;
			const int eps=M*0.2;
			int histo[N] = {0};

			for(int i=0; i<M*N; ++i){
				int idx = r.uniform(N);
				++histo[idx];
			}

			for(int i=0; i<N; ++i){
				int cnt = histo[i];
				//printf("%d\n", cnt);
				assert(M-eps < cnt && cnt < M+eps);
			}
		}
	}


	{
		al::Plane<double> p;
		p.fromNormalAndPoint(Vec3d(1,0,0), Vec3d(1,1,1));

		assert(p.distance(Vec3d(1,0,0)) ==  0);
		assert(p.distance(Vec3d(0,1,0)) == -1);
		assert(p.distance(Vec3d(0,0,1)) == -1);
		assert(p.distance(Vec3d(2,0,0)) ==  1);
	}

	{
		Frustumd f;
		f.fbl = Vec3d(-1,-1,-1);
		f.fbr = Vec3d( 1,-1,-1);
		f.ftl = Vec3d(-1, 1,-1);
		f.ftr = Vec3d( 1, 1,-1);
		f.nbl = Vec3d(-1,-1, 1);
		f.nbr = Vec3d( 1,-1, 1);
		f.ntl = Vec3d(-1, 1, 1);
		f.ntr = Vec3d( 1, 1, 1);
		f.computePlanes();

//		Plane<double>& p = f.pl[Frustumd::LEFT];
//		printf("%g %g %g\n", p.normal()[0], p.normal()[1], p.normal()[2]);
//
//		Vec3d nrm = cross(f.fbl-f.nbl, f.ntl-f.nbl);
//		printf("%g %g %g\n", nrm[0], nrm[1], nrm[2]);
//
//		printf("%g %g %g\n", (f.fbl-f.nbl)[0], (f.fbl-f.nbl)[1], (f.fbl-f.nbl)[2]);
//		printf("%g %g %g\n", (f.ntl-f.nbl)[0], (f.ntl-f.nbl)[1], (f.ntl-f.nbl)[2]);

		assert(f.testPoint(Vec3d(0,0,0)) == Frustumd::INSIDE);
		assert(f.testPoint(Vec3d(2,1,1)) == Frustumd::OUTSIDE);

		assert(f.testSphere(Vec3d(0,0,0), 0.9) == Frustumd::INSIDE);
		assert(f.testSphere(Vec3d(0,0,0), 1.1) == Frustumd::INTERSECT);
		assert(f.testSphere(Vec3d(2,2,2), 0.5) == Frustumd::OUTSIDE);
	}

	return 0;
}


