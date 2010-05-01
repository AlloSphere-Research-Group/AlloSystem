#include "utAllocore.h"

inline bool eq(double x, double y, double eps=0.000001){ return abs(x-y) < eps; }

int utMath(){


	// Vec
	{
		const int N = 4;

		// Should be able to hold generic objects not having overloaded operators
		{ Vec<1, Vec<1, int> > t; }

		Vec<N,double> a, b, c;	assert(a.size() == N);

		a[0] = 0;				assert(a[0] == 0);
								assert(a.elems[0] == 0);

		a.set(1);				assert(a == 1);
		b.set(a);				assert(b == 1);

		{
		a.set(1);
		b.set(0);
		double * p = a.ptr();	assert(p[0] == a[0]);
		b.set(p);				assert(a == b);

		char c1[] = {4,4,4,4};
		a.set(c1);				assert(a == 4);
		
		char c2[] = {1,0,1,0,1,0,1,0};
		a.set(c2,2);			assert(a == 1);
		}
		
		a.zero();				assert(a == 0);

		a = 1;					assert(a == 1);
								assert(!(a != 1));
		b = a;					assert(b == a);
								assert(!(b != a));
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
		a = 1. / a;				assert(a == 1./6);

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
		assert(a.magSqr() == N);
		assert(b.magSqr() == N);
		assert(a.norm1() == N);
		assert(a.norm2() == sqrt(N));
			
		a.set(1).negate();		assert(a == -1);
		a.set(1).normalize();	assert(a == 1./sqrt(N));
		assert(a == b.set(10).sgn());
		
		b = a = 1;
		assert(concat(a,b) == 1);
		
		// conversion
		{
		a = 0;
		Vec<N+1, double> t = concat(a, Vec<1,char>(1));
		assert(t.size() == a.size()+1);
		}
		
		{
		for(int i=0; i<a.size(); ++i) a[i]=i;
		
		Vec<2, double> t;
		t = sub<2>(a);			assert(t[0] == 0 && t[1] == 1);
		t = sub<2>(a,2);		assert(t[0] == 2 && t[1] == 3);
		}
		
		assert(angle(Vec3d(1,0,0), Vec3d(1,0,0)) == 0);
		assert(angle(Vec3d(1,0,0), Vec3d(0,1,0)) == M_PI_2);
		assert(angle(Vec3d(1,0,0), Vec3d(0,-1,0)) == M_PI_2);
		
		{
		Vec3d r;
		centroid3(r, Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(0,0,1));
		assert(r == 1/3.);
		
		normal(r, Vec3d(1,0,0), Vec3d(0,1,0), Vec3d(-1,0,0));
		assert(r == Vec3d(0,0,1));
		
		Vec3d pos(1,2,3);
		Vec3d to(4,5,6);
		Vec3d rel = to - pos;
		
		assert(rel[0]==3 && rel[1]==3 && rel[2]==3);
		
		}
		
		a = 0;
		b = 1;
		assert(vmin(a,b) == 0);
		assert(vmax(a,b) == 1);
		
	}
	
	
	// Vec3
	{
		Vec3d a, b, c;

		a.set(1,0,0);
		b.set(0,1,0);
		c.set(0,0,1);
			assert(c == cross(a,b));
			assert(c == (a^b));

		a = b;
			assert(a == b);
	}
	
	
	// Mat
	{
		Mat<3,double> a;//, b;
		
		//a(0,1) = a(1,0);
		#define CHECK(m, a,b,c, d,e,f, g,h,i)\
		assert(m(0,0)==a); assert(m(0,1)==b); assert(m(0,2)==c);\
		assert(m(1,0)==d); assert(m(1,1)==e); assert(m(1,2)==f);\
		assert(m(2,0)==g); assert(m(2,1)==h); assert(m(2,2)==i)
		
		a.identity();	CHECK(a, 1,0,0, 0,1,0, 0,0,1);
		
		assert(a.trace() == 3);
		
		a += 2;		CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a -= 1;		CHECK(a, 2,1,1, 1,2,1, 1,1,2);
		a *= 2;		CHECK(a, 4,2,2, 2,4,2, 2,2,4);
		a /= 2;		CHECK(a, 2,1,1, 1,2,1, 1,1,2);
		
		a.identity();
		a = a+2;	CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a = a-1;	CHECK(a, 2,1,1, 1,2,1, 1,1,2);
		a = a*2;	CHECK(a, 4,2,2, 2,4,2, 2,2,4);
		a = a/2;	CHECK(a, 2,1,1, 1,2,1, 1,1,2);

		a.identity();
		a = 2.+a;	CHECK(a, 3,2,2, 2,3,2, 2,2,3);
		a = 4.-a;	CHECK(a, 1,2,2, 2,1,2, 2,2,1);
		a = 2.*a;	CHECK(a, 2,4,4, 4,2,4, 4,4,2);

		#undef CHECK
	}


	{	Complexd c(0,0);
		#define T(x, y) assert(x == y);
		T(c, Complexd(0,0))
		c.fromPolar(1, 0.2);	T(c, Polard(0.2))
		c.fromPhase(2.3);		T(c, Polard(2.3))
		T(c != Complexd(0,0), true)
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

//	{	Quatd q(0,0,0,0);
//		#define T(x, y) assert(x == y);
//		T(q, Quatd(0,0,0,0))
//		T(q.conj(), Quatd(q.r, -q.i, -q.j, -q.k))
//		#undef T
//	}



	// Functions
	{
	const double pinf = 1e800;			// + infinity
	const double ninf = -pinf;			// - infinity

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

	#define T(x, y) assert(al::floor(x) == y);
	T(0., 0.)	T( 1., 1.) T( 1.2, 1.) T( 1.8, 1.) T( 1000.1, 1000.)
				T(-1.,-1.) T(-1.2,-2.) T(-1.8,-2.) T(-1000.1,-1001.)
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

	#define T(x) assert(eq(al::pow64(x), x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x*x, 10));
	T(0.) T(1.) T(1.01) T(1.02) T(-1.) T(-1.01) T(-1.02)
	#undef T

//	#define T(x,r) assert(al::powerOf2(x) == r);
//	T(0, false) T(1, true) T(2, true) T(3, false) T(4, true)
//	#undef T
	
	#define T(x,y,r) assert(al::remainder(x,y) == r);
	T(7,7,0) T(7,1,0) T(7,4,3) T(7,3,1) T(14,3,2)
	#undef T
	
	#define T(x,y) assert(al::round(x) == y);
	T(0.f, 0.f) T(0.2f, 0.f) T(0.8f, 1.f) T(-0.2f, 0.f) T(-0.8f,-1.f) T(0.5f, 0.f) T(-0.5f, 0.f)
	T(0.0, 0.0) T(0.20, 0.0) T(0.80, 1.0) T(-0.20, 0.0) T(-0.80,-1.0) T(0.50, 0.0) T(-0.50, 0.0)
	#undef T

	#define T(x,y,r) assert(al::round(x,y) == r);
	T(0.0,0.1, 0.0) T(0.1,0.1, 0.1) T(0.15,0.1, 0.1) T(-0.15,0.1, -0.1)
	#undef T

	#define T(x,y,r) assert(al::roundAway(x,y) == r);
	T(0.0,0.1, 0.0) T(0.1,0.1, 0.1) T(0.15,0.1, 0.2) T(-0.15,0.1, -0.2)
	#undef T

	#define T(x,r) assert(al::sgn(x) == r);
	T(0., 1.) T(0.1, 1.) T(-0.1, -1.)
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

	#define T(x, y) assert(eq(al::wrap(x, 1., -1.), y));
	T(0., 0.)	T( 0.5, 0.5) T( 1.,-1.) T( 1.2,-0.8) T( 2.2, 0.2)
				T(-0.5,-0.5) T(-1.,-1.) T(-1.2, 0.8) T(-2.2,-0.2)
	#undef T
	}


	// Generators
	{
		{
		al::gen::Val<int> g(0);
		assert(g() == 0);
		assert(g() == 0);
		g = 1;
		assert(g() == 1);
		}{
		al::gen::RAdd<int> g(1,0);
		assert(g() == 0);
		assert(g() == 1);
		assert(g() == 2);
		assert(g() == 3);
		}{
		al::gen::RMul<int> g(2,1);
		assert(g() == 1);
		assert(g() == 2);
		assert(g() == 4);
		assert(g() == 8);
		assert(g() ==16);
		}{
		al::gen::RMulAdd<int> g(2,1,1);
		assert(g() == 1);
		assert(g() == 3);
		assert(g() == 7);
		assert(g() ==15);
		assert(g() ==31);
		}
	}


	// Random
	{
		using namespace al::rnd;
		//for(int i=0; i<8; ++i) printf("%u\n", al::rnd::seed());
		assert(seed() != seed() != seed() != seed());
		
		{
		LinCon r;
		assert(r() != r() != r() != r());
		}

		{
		MulLinCon r;
		assert(r() != r() != r() != r());
		}

		{
		Tausworthe r;
		assert(r() != r() != r() != r());
		}
		
		//assert(uniform() != uniform() != uniform());

		Random<> r;
		int N = 100000;
		for(int i=0; i<N; ++i){ int v=r.uniform(20, 0); assert(v < 20 && v >= 0); }
		for(int i=0; i<N; ++i){ int v=r.uniform(20,10); assert(v < 20 && v >=10); }
		for(int i=0; i<N; ++i){ int v=r.uniform(20,-10); assert(v < 20 && v >=-10); }

		//for(int i=0; i<32; ++i) printf("% g ", r.uniformS());
		//for(int i=0; i<32; ++i) printf("%d ", r.prob(0.1));
		//for(int i=0; i<128; ++i) printf("% g\n", r.gaussian());
		printf("\n");
	}

	return 0;
}


