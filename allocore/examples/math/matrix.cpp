/*
AlloCore Example: Matrices

Description:
This shows basic usage of the Mat class which represents an N x N square matrix.

Author:
Lance Putnam, June 2016
*/

#include "allocore/math/al_Mat.hpp"
using namespace al;

int main(){

	// Lesson 1: Declarations
	// =========================================================================
	{
	/*
	Mat is a template class with two template parameters:
	1) N, the number of rows/columns and 2) T, the element type.
	Here are some example declarations:
	*/
		Mat<3,float> mat3f;		// 3x3 matrix of floats
		Mat<2,int> mat2i;		// 2x2 matrix of ints
		Mat<11,double> mat11d;	// 11x11 matrix of doubles (!)
		Mat<1,float> mat1f;		// 1x1 (scalar) with a float
		Mat<0,float> mat0f;		// even 0x0 matrices are allowed, for completeness

	/*
	For convience, some common types are predefined:
	*/
	//	2x2          3x3          4x4
		{ Mat2f v; } { Mat3f v; } { Mat4f v; } // float
		{ Mat2d v; } { Mat3d v; } { Mat4d v; } // double
		{ Mat2i v; } { Mat3i v; } { Mat4i v; } // int
	}



	// Lesson 2: Creation
	// =========================================================================
	/*
	Mat has many different kinds of constructors. Here are some examples:
	*/
	{	Mat3f x;				// 0 0 0
								// 0 0 0
								// 0 0 0
		Mat3f b(				
			1,2,3,				// 1 2 3
			4,5,6,				// 4 5 6
			7,8,9				// 7 8 9
		);

		Mat3f c(b);				// Copy values from another matrix
		Mat3d d(b);				// Different types work as well, but sizes must match
	/*
	Mat can be initialized from C-arrays. Elements are initialized in
	column-major order.
	*/
		float arr[] = {1,2,3, 4,5,6, 7,8,9};
		Mat3f g(arr);			// Initialize from C-array, sizes match
		Mat3i h(arr);			// The C-array and matrix can be different types

	/*
	There are also factory functions that return special types of matrices,
	e.g., transformation matrices
	*/
		x =  Mat3f::identity();				/* 1 0 0
											   0 1 0
											   0 0 1 */

		x = Mat3f::scaling(2,3);			/* 2 0 0
											   0 3 0
											   0 0 1 */

		x = Mat3f::scaling(2);				/* 2 0 0
											   0 2 0
											   0 0 1 */

		x = Mat3f::translation(Vec2f(2,3));	/* 1 0 2
											   0 1 3
											   0 0 1 */
		
		Mat4f y;

		// Rotations are specified from an angle and the two bases of the plane
		y = Mat4f::rotation(M_PI/2, 0,1);	/* 0 -1  0  0
											   1  0  0  0
											   0  0  1  0
											   0  0  0  1*/

		y = Mat4f::rotation(M_PI/2, 1,2);	/* 1  0  0  0
											   0  0 -1  0
											   0  1  0  0
											   0  0  0  1*/
	}



	// Lesson 3: Acccessing Elements
	// =========================================================================
	/*
	All matrices know their size (number of elements):
	*/
	{	Mat3f a;
		a.size();				// Returns '9'
	}

	/*
	Elements are accessed using a 2D lookup (row, column):
	*/
	{	Mat3f a(
			10,20,30,
			40,50,60,
			70,80,90
		);

		a(0,0);					// 10
		a(1,0);					// 40
		a(2,1);					// 80
	}
	/*
	Elements can also be accessed in column-major order using normal array access:
	*/
	{	Mat2f a(10,20, 30,40);
		a[0];					// 10
		a[2];					// 30
	}
	/*
	The 'set' family of methods make it easy to set multiple elements at once:
	*/
	{	Mat2f a, b;
		a.set(1,2, 3,4);		/* 1 2
								   3 4 */

		b.set(1);				/* 1 1
								   1 1 */

		a.set(b);				// Sets 'a' to 'b'

		float arr[] = {4,3,2,1};
		a.set(arr);				/* 4 2
								   3 1 */
	}
	/*
	We can pun C-arrays into Mats to help in interfacing with other code
	*/
	{
		float arr[4];			// An array
		Mat2f::pun(arr).setIdentity();
		arr;					// Array values are {1,0,0,1}
		//for(int i=0; i<4; ++i) printf("%f\n", arr[i]);
	}
	/*
	We can get groups of elements as vectors (al::Vec):
	*/
	{	Mat3f a(
			1,2,3,
			4,5,6,
			7,8,9
		);

		a.col(0);				// First column {1,4,7}
		a.row(1);				// Second row {4,5,6}
		a.diagonal();			// Diagonal {1,5,9}
	}
	/*
	Submatrices can be obtained:
	*/
	{	Mat3f a(
			1,2,3,
			4,5,6,
			7,8,9
		);

		// submatrix returns a matrix with one one row, column removed
		Mat2f b;
		b = a.submatrix(2,2);	/* 1 2
								   4 5*/

		b = a.submatrix(0,1);	/* 4 6
								   7 9*/
	}



	// Lesson 4: Arithmetic
	// =========================================================================
	/*
	Mat has the expected arithmetic operators defined:
	*/
	{	Mat2f a(
			1,2,
			3,4
		);

		-a;						/* -1 -2
								   -3 -4 */

		a + 1;					/*  2  3
								    4  5 */

		a - 1;					/*  0  1
								    2  3 */

		a * 3;					/*  3  6
								    9 12 */

		a / 4;					/*  0.25 0.50
								    0.75 1.00 */

		a += 1;					// And all the compound operators...
		a -= 1;
		a *= 2;
		a /= 2;


	}
	/*
	Operations between matrices are defined:
	*/
	{	Mat2f a(
			1,2,
			3,4
		);

		Mat2f b(
			10,20,
			30,40
		);

		a + b;
		a - b;
		a * b;
		a += b;
		a -= b;
		a *= b;
	}



	// Lesson 5: Matrix Functions
	// =========================================================================
	/*
	Basic matrix operations:
	*/
	{	Mat2f a(
			1,2,
			3,4
		);

		a.transpose();			/*  1  3
								    2  4 */

		a.trace();				// Sum along diagonal: 5

		determinant(a);			// Returns determinant
		invert(a);				// Inverts matrix
	}
	/*
	We can also apply transformations to create/transform homogenous matrices:
	*/
	{
		Mat4f a;
		a.setIdentity();
		a.scale(10,20,30);		// Non-uniform scaling
		a.scale(-2);			// Uniform scaling
		a.translate(-1,-1,-1);	// Translate
	}
}
