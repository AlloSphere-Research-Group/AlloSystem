/*
AlloCore Example: Vectors

Description:
This shows basic usage of the Vec class which represents an n-dimensional vector
or n-vector.

Author:
Lance Putnam, 10/2012, putnam.lance@gmail.com
*/

#include "allocore/math/al_Vec.hpp"
using namespace al;

int main(){

	// Lesson 1: Declarations
	// =========================================================================
	{
	/*
	Vec is a template class with two template parameters:
	1) N, the number of dimensions and 2) T, the element type.
	Here are some example declarations:
	*/
		Vec<3,float> vec3f;		// 3-vector of floats
		Vec<2,int> vec2i;		// 2-vector of ints
		Vec<11,double> vec11d;	// 11-vector of doubles (!)
		Vec<1,float> vec1f;		// 1-vector (scalar) with a float
		Vec<0,float> vec0f;		// even 0-vectors are allowed, for completeness

	/*
	For convience, some common types are predefined:
	*/
	//	2-vector     3-vector     4-vector
		{ Vec2f v; } { Vec3f v; } { Vec4f v; } // float
		{ Vec2d v; } { Vec3d v; } { Vec4d v; } // double
		{ Vec2i v; } { Vec3i v; } { Vec4i v; } // int
	}



	// Lesson 2: Constructors
	// =========================================================================
	/*
	Vec has many different kinds of constructors. Here are some examples:
	*/
	{	Vec3f x;				// Initialize to {0, 0, 0}
		Vec3f a(1);				// Initialize to {1, 1, 1}
		Vec3f b(1,2,3);			// Initialize to {1, 2, 3}
		Vec2f c(4,5);			// Initialize to {4, 5}
		Vec4f d(6,7,8,9);		// Init'ing with scalars works up to 4-vectors

		Vec2d e(c);				// Initialize from vector to {4, 5}
		Vec3f f(c, 6);			// Initialize from vector and scalar to {4,5,6}

	/*
	Vec can be initialized from C-arrays:
	*/
		float arr2[] = {1,2};
		Vec2f g(arr2);			// Initialize from C-array, sizes match
		Vec2i h(arr2);			// The C-array and vector can be different types
	/*
	Sizes can be different, but the C-array must be longer. Only the first
	n elements are copied.
	*/
		float arr3[] = {3,4,5};
		Vec2f i(arr3);
	/*
	It is also possible to specify a stride amount through the C-array
	*/
		float arr8[] = {0,1,2,3,4,5,6,7};
		Vec3f l(arr8, 2);		// Initialize to {0, 2, 4}
		Vec3f m(arr8, 3);		// Initialize to {0, 3, 6}
	}



	// Lesson 3: Acccessing Elements
	// =========================================================================
	/*
	All vectors know their size (number of elements):
	*/
	{	Vec3f a;
		a.size();				// Returns '3'
	}
	/*
	Access is accomplished using the normal array access operator:
	*/
	{	Vec3f a(7,2,5);
		a[0];					// Access element 0
		a[2];					// Access element 2
	}
	/*
	For 1-, 2-, 3-, and 4-vectors, it is possible to use symbolic names. The
	symbols 'x', 'y', 'z', and 'w' are used to access elements 0, 1, 2, and 3,
	respectively. E.g.:
	*/
	{	Vec2f b(1,2);
		b.x = b.y;				// Results in {2, 2}

		Vec3f c(1,2,3);
		c.x = c.y = c.z;		// Results in {3, 3, 3}

		Vec4f d(1,2,3,4);
		d.x = d.y = d.z = d.w;	// Results in {4, 4, 4, 4}
	}
	/*
	The 'set' family of methods make it easy to set multiple elements at once:
	*/
	{	Vec3f a, b;
		a.set(1,2,3);			// Sets 'a' to {1, 2, 3}
		b = 1;					// Sets 'b' to {1, 1, 1}
		a = b;					// Sets 'a' to {1, 1, 1}

		Vec2f c(9,1);
		a.set(c, 3);			// Sets 'a' to {9, 1, 3}

		float arr[] = {1,7,2,8,3,9};
		a.set(arr);				// Sets 'a' to {1, 7, 2}
		a.set(arr,2);			// Sets 'a' to {1, 2, 3} by using a stride of 2
	}
	/*
	It is easy to obtain subvectors, contiguous parts of a larger vector:
	*/
	{
		Vec4f a(7,5,0,2);
		a.sub<2>();				// Returns 2-vector {7, 5}
		a.sub<2,2>();			// Returns 2-vector {0, 2}
		a.sub<3,1>();			// Returns 3-vector {5, 0, 2}
	}
	/*
	It is also possible to "swizzle", that is, obtain a new vector comprised of
	arbitrary elements from a particular vector:
	*/
	{	Vec3f a(9,3,7);
		a.get(0,0,0);			// Returns {9, 9, 9}
		a.get(2,1,0);			// Returns {7, 3, 9}
		a.get(1,0);				// Returns {3, 9}
	}



	// Lesson 4: Arithmetic
	// =========================================================================
	/*
	Like expected of any good vector, Vec has many arithemtic operators defined:
	*/
	{	Vec3f a(2,6,4);
		-a;						// Returns {-2, -6, -4}
		a + 1;					// Returns {3, 7, 5}
		a - 1;					// Returns {1, 5, 3}
		a * 3;					// Returns {6, 18, 12}
		a / 4;					// Returns {0.5, 1.5, 1}
		a += 1;					// Sets 'a' to {3, 7, 5}
		a -= 1;					// Sets 'a' to {2, 6, 4}
		a *= 2;					// Sets 'a' to {4,12, 8}
		a /= 2;					// Sets 'a' to {2, 6, 4}
	}
	/*
	Operations between vectors are element-wise:
	*/
	{	Vec3f a(2,6,4);
		Vec3f b(4,1,2);
		a + b;					// Returns {6, 7, 6}
		a - b;					// Returns {-2, 5, 2}
		a * b;					// Returns {8, 6, 8}
		a / b;					// Returns {0.5, 6, 2}
		a += b;					// Sets 'a' to {6, 7, 6}
		a -= b;					// Sets 'a' to {2, 6, 4}
		a *= b;					// Sets 'a' to {8, 6, 8}
		a /= b;					// Sets 'a' to {2, 6, 4}
	}
	/*
	Some comparison operators are provided:
	*/
	{	Vec3f a(2,6,4);
		Vec3f b(2,2,2);
		Vec3f c(2,6,4);

		a == b;					// Returns 'false'
		a == c;					// Returns 'true'
		a != b;					// Returns 'true'
		a != c;					// Returns 'false'
	}



	// Lesson 5: Vector Functions
	// =========================================================================
	/**/
	{	Vec2f v(3,4);

		v.mag();				// Returns 5, the magnitude of the vector
		v.magSqr();				// Returns 25, the magnitude squared

		v.normalize();			// Sets magnitude to 1 without changing the
								// vector's direction; the map a <- a / a.mag().

		v.normalize(0.5);		// Sets magnitude to 0.5 without changing the
								// direction

		v.normalized();			// Returns closest vector on unit sphere

		Vec3f A( 1, 2, 3);
		Vec3f B( 3,-2, 0);

		A.dot(B);				// Returns -1 (=1*3 + 2*-2 + 3*0);
								// the dot product of A and B

		A.product();			// Returns 6 (= 1*2*3), the product of all elems
		B.sum();				// Returns 1 (= 3+-2+0), the sum of all elems

		cross(A, B);			// Returns cross product A x B (3-vectors only)
		angle(A, B);			// Returns angle, in radians, between A and B
		dist(A, B);				// Returns Euclidean distance between A and B;
								// same as (A-B).mag()

		min(A, B);				// Returns {1,-2, 0}; minimum values between A and B
		max(A, B);				// Returns {3, 2, 3}; maximum values between A and B
	}
}
