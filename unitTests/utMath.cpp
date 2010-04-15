#include "utAllocore.h"

int utMath(){

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
		}
		
		a = 0;
		b = 1;
		assert(vmin(a,b) == 0);
		assert(vmax(a,b) == 1);
		
	}
	
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
	
	{
		Mat<3,double> a;
		
		a(0,1) = a(1,0);
	}

	return 0;
}


