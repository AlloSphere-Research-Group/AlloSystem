#ifndef INCLUDE_AL_MAT_HPP
#define INCLUDE_AL_MAT_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, 
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright 
		notice, this list of conditions and the following disclaimer in the 
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its 
		contributors may be used to endorse or promote products derived from 
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Generic fixed-size n-by-n matrix

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/


#include <cmath>
#include <stdio.h>
#include "allocore/math/al_Vec.hpp"

namespace al {

template <int N, class T> class Mat;

typedef Mat<3,float>	Mat3f;	///< float 3x3 matrix
typedef Mat<3,double>	Mat3d;	///< double 3x3 matrix
typedef Mat<3,int>		Mat3i;	///< integer 3x3 matrix
typedef Mat<4,float>	Mat4f;	///< float 4x4 matrix
typedef Mat<4,double>	Mat4d;	///< double 4x4 matrix
typedef Mat<4,int>		Mat4i;	///< integer 4x4 matrix

// Forward iterates from 0 to n-1. Current index is 'i'.
#define IT(n) for(int i=0; i<(n); ++i)


/// Flag type to prevent element initialization
static struct MatNoInit{} MAT_NO_INIT;


/// Fixed-size n-by-n square matrix

/// Elements are stored in column-major format.
///
template <int N, class T>
class Mat{
public:

	/// Column-major array of elements
	T mElems[N*N];


	//--------------------------------------------------------------------------
	// Constructors

	/// Default constructor that initializes elements to zero
	Mat(){ set(T(0)); }

	/// @param[in] arr	one dimensional array in column-major
	Mat(const T * arr){ set(arr); }
	
	/// @param[in] src	matrix with same dimension, but possibly different type
	template <class U>
	Mat(const Mat<N,U>& src){ set(src.elems()); }

	/// Construct without initializing elements
	Mat(const MatNoInit& v){}

	/// 2x2 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2,
		const T& r2c1, const T& r2c2
	){
		set(r1c1, r1c2,
			r2c1, r2c2
		);
	}

	/// 3x3 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2, const T& r1c3,
		const T& r2c1, const T& r2c2, const T& r2c3,
		const T& r3c1, const T& r3c2, const T& r3c3
	){
		set(r1c1, r1c2, r1c3,
			r2c1, r2c2, r2c3,
			r3c1, r3c2, r3c3
		);
	}

	/// 4x4 matrix constructor with element initialization
	Mat(
		const T& r1c1, const T& r1c2, const T& r1c3, const T& r1c4,
		const T& r2c1, const T& r2c2, const T& r2c3, const T& r2c4,
		const T& r3c1, const T& r3c2, const T& r3c3, const T& r3c4,
		const T& r4c1, const T& r4c2, const T& r4c3, const T& r4c4
	){
		set(r1c1, r1c2, r1c3, r1c4,
			r2c1, r2c2, r2c3, r2c4,
			r3c1, r3c2, r3c3, r3c4,
			r4c1, r4c2, r4c3, r4c4
		);
	}



	//--------------------------------------------------------------------------
	// Factory Methods

	/// Get identity matrix
	static Mat identity(){
		return Mat(MAT_NO_INIT).setIdentity();
	}

	/// Get a rotation transform matrix
	static Mat rotation(double angle, int dim1, int dim2){
		double cs = cos(angle);
		double sn = sin(angle);

		Mat m(MAT_NO_INIT);
		m.setIdentity();
		m(dim1,dim1) = cs;
		m(dim1,dim2) =-sn;
		m(dim2,dim1) = sn;
		m(dim2,dim2) = cs;
		return m;
	}

	/// Get a scaling transform matrix
	template <class V>
	static Mat scaling(const Vec<N-1,V>& v){
		Mat m;
		for(int r=0; r<N-1; ++r) m(r,r) = v[r];
		m(N-1,N-1) = T(1);
		return m;
	}
	
	/// Get a translation transform matrix
	template <class V>
	static Mat translation(const Vec<N-1,V>& v){
		Mat m(MAT_NO_INIT);
		m.setIdentity();
		for(int r=0; r<N-1; ++r) m(r,N-1) = v[r];
		return m;
	}



	//--------------------------------------------------------------------------
	// Memory Operations

	/// Returns C array type punned into a matrix
	static Mat& pun(T * src){ return *(Mat*)(src); }

	/// Returns total number of elements
	static int size(){ return N*N; }

	/// Get read-only pointer to elements
	const T* elems() const { return mElems; }
	
	/// Get read-write pointer to elements
	T* elems(){ return mElems; }

	/// Set element at index with no bounds checking
	T& operator[](int i){ return elems()[i];}
	
	/// Get element at index with no bounds checking
	const T& operator[](int i) const { return elems()[i]; }

	/// Set element at row i, column j
	T& operator()(int i, int j){ return (*this)[j*N+i]; }
	
	/// Get element at row i, column j
	const T& operator()(int i, int j) const { return (*this)[j*N+i]; }

	/// Return column i as vector
	Vec<N,T> col(int i) const { return Vec<N,T>(elems() + i*N); }

	/// Return row i as vector
	Vec<N,T> row(int i) const { return Vec<N,T>(elems()+i, N); }
	
	/// Return diagonal
	Vec<N,T> diagonal() const { return Vec<N,T>(elems(), N+1); }

	/// Transpose elements
	Mat& transpose(){
		for(int j=0; j<N-1; ++j){	// row and column
		for(int i=j+1; i<N; ++i){	// offset into row or column
			T& a = (*this)(i,j);
			T& b = (*this)(j,i);
			T c = a; a = b;	b = c;	// swap elements
		}} return *this;
	}

	/// Get a submatrix
	template <int M>
	Mat<M,T> sub(int row=0, int col=0) const {
		Mat<M,T> res(MAT_NO_INIT);
		for(int j=0; j<M; ++j){
		for(int i=0; i<M; ++i){
			res(j,i) = (*this)(j+row, i+col);
		}}
	}

	/// Return matrix punned as a vector
	Vec<N*N,T>& vec(){ return *(Vec<N*N,T>*)(this); }


	//--------------------------------------------------------------------------
	// Basic Arithmetic Operations

	Mat& operator *= (const Mat& v){ return multiply(*this, Mat(*this),v); }
	Mat& operator += (const Mat& v){ IT(size()){ (*this)[i] += v[i]; } return *this; }
	Mat& operator -= (const Mat& v){ IT(size()){ (*this)[i] -= v[i]; } return *this; }
	Mat& operator += (const T& v){ IT(size()){ (*this)[i] += v; } return *this; }
	Mat& operator -= (const T& v){ IT(size()){ (*this)[i] -= v; } return *this; }
	Mat& operator *= (const T& v){ IT(size()){ (*this)[i] *= v; } return *this; }
	Mat& operator /= (const T& v){ IT(size()){ (*this)[i] /= v; } return *this; }

	Mat operator - () const { Mat r(MAT_NO_INIT); IT(size()){ r[i]=-(*this)[i]; } return r; }
	Mat operator + (const Mat& v) const { return Mat(*this) += v; }
	Mat operator - (const Mat& v) const { return Mat(*this) -= v; }
	Mat operator * (const Mat& v) const { return Mat(*this) *= v; }
	Mat operator + (const T& v) const { return Mat(*this) += v; }
	Mat operator - (const T& v) const { return Mat(*this) -= v; }
	Mat operator * (const T& v) const { return Mat(*this) *= v; }
	Mat operator / (const T& v) const { return Mat(*this) /= v; }

	/// Computes matrix product r = a * b
	
	/// Returns reference to result
	///
	static Mat& multiply(Mat& r, const Mat& a, const Mat& b){
		for(int j=0; j<N; ++j){
			const Vec<N,T>& bcol = b.col(j);
			for(int i=0; i<N; ++i){
				r(i,j) = a.row(i).dot(bcol);
			}
		}
		return r;
	}
	
	/// Computes product of matrix multiplied by column vector, r = m * vCol
	static Vec<N,T>& multiply(Vec<N,T>& r, const Mat& m, const Vec<N,T>& vCol){
		IT(N){ r[i] = m.row(i).dot(vCol); }
		return r;
	}

	/// Computes product of row vector multiplied by matrix, r = vRow * m
	static Vec<N,T>& multiply(Vec<N,T>& r, const Vec<N,T>& vRow, const Mat& m){
		IT(N){ r[i] = vRow.dot(m.col(i)); }
		return r;
	}

	/// Set all elements to value
	Mat& set(const T& v){ IT(size()){ (*this)[i]=v; } return *this; }
	
	/// Set elements in column-major order from C array
	template <class U>
	Mat& set(const U * arr){ IT(size()){ (*this)[i]=arr[i]; } return *this; }

	/// Set elements in column-major order from C array
	
	/// @param[in] arr			1D array from which to copy (stride=1)
	/// @param[in] numElements	number of elements to copy
	/// @param[in] matOffset	index offset into matrix
	/// @param[in] matStride	amount to stride through matrix
	template <class U>
	Mat& set(const U * arr, int numElements, int matOffset, int matStride=1){
		IT(numElements){ (*this)[i*matStride+matOffset]=arr[i]; } return *this;
	}

	/// Set 2-by-2 (sub)matrix from arguments	
	Mat& set(
		const T& r1c1, const T& r1c2,
		const T& r2c1, const T& r2c2,
		int row=0, int col=0
	){
		setCol2(r1c1, r2c1, col  ,row);
		setCol2(r1c2, r2c2, col+1,row); return *this;
	}

	/// Set 3-by-3 (sub)matrix from arguments	
	Mat& set(
		const T& r1c1, const T& r1c2, const T& r1c3,
		const T& r2c1, const T& r2c2, const T& r2c3,
		const T& r3c1, const T& r3c2, const T& r3c3,
		int row=0, int col=0
	){
		setCol3(r1c1, r2c1, r3c1, col  ,row);
		setCol3(r1c2, r2c2, r3c2, col+1,row);
		setCol3(r1c3, r2c3, r3c3, col+2,row); return *this;
	}
	
	/// Set 4-by-4 (sub)matrix from arguments
	Mat& set(
		const T& r1c1, const T& r1c2, const T& r1c3, const T& r1c4,
		const T& r2c1, const T& r2c2, const T& r2c3, const T& r2c4,
		const T& r3c1, const T& r3c2, const T& r3c3, const T& r3c4,
		const T& r4c1, const T& r4c2, const T& r4c3, const T& r4c4,
		int row=0, int col=0
	){		
		setCol4(r1c1, r2c1, r3c1, r4c1, col  ,row);
		setCol4(r1c2, r2c2, r3c2, r4c2, col+1,row);
		setCol4(r1c3, r2c3, r3c3, r4c3, col+2,row);
		setCol4(r1c4, r2c4, r3c4, r4c4, col+3,row); return *this;
	}

	/// Set a (sub)column
	Mat& setCol2(const T& v1, const T& v2, int col=0, int row=0){
		(*this)(row  , col) = v1;
		(*this)(row+1, col) = v2; return *this;
	}

	/// Set a (sub)column
	Mat& setCol3(const T& v1, const T& v2, const T& v3, int col=0, int row=0){
		(*this)(row  , col) = v1;
		(*this)(row+1, col) = v2;
		(*this)(row+2, col) = v3; return *this;
	}

	/// Set a (sub)column	
	Mat& setCol4(const T& v1, const T& v2, const T& v3, const T& v4, int col=0, int row=0){
		(*this)(row,   col) = v1;
		(*this)(row+1, col) = v2;
		(*this)(row+2, col) = v3;
		(*this)(row+3, col) = v4; return *this;
	}

	/// Set elements on diagonal to one and all others to zero
	Mat& setIdentity(){
		for(int i=0; i<N; ++i)
			(*this)[i*(N+1)] = T(1);

		for(int i=0;   i<N-1  ; ++i){
		for(int j=i+1; j<N+i+1; ++j){
			(*this)[i*N + j] = T(0);
		}}
		return *this;
	}



	//--------------------------------------------------------------------------
	// Linear Operations

	/// Get trace (sum of diagonal elements)
	T trace() const { return diagonal().sum(); }

	// Affine transformations

	/// Rotate transformation matrix on a local plane (A' = AR)

	/// @param[in] angle	angle of rotation, in radians
	/// @param[in] dim1		local coordinate frame axis to rotate away from
	/// @param[in] dim2		local coordinate frame axis to rotate towards
	Mat& rotate(double angle, int dim1, int dim2){
		double cs = cos(angle);
		double sn = sin(angle);
		for(int R=0; R<N-1; ++R){
			const T& v1 = (*this)(R, dim1);
			const T& v2 = (*this)(R, dim2);
			T t = v1*cs + v2*sn;
			(*this)(R, dim2) = v2*cs - v1*sn;
			(*this)(R, dim1) = t;
		}
		return *this;
	}

	/// Rotate submatrix on a global plane (A' = RA)
	template <int M>
	Mat& rotateGlobal(double angle, int dim1, int dim2){
		double cs = cos(angle);
		double sn = sin(angle);
		for(int C=0; C<M; ++C){
			const T& v1 = (*this)(dim1, C);
			const T& v2 = (*this)(dim2, C);
			T t = v1*cs - v2*sn;
			(*this)(dim2, C) = v2*cs + v1*sn;
			(*this)(dim1, C) = t;
		}
		return *this;
	}
	
	/// Rotate transformation matrix on a global plane (A' = RA)
	Mat& rotateGlobal(double angle, int dim1, int dim2){
		return rotateGlobal<N-1>(angle,dim1,dim2); }

	/// Scale transformation matrix
	template<class V>
	Mat& scale(const Vec<N-1,V>& amount){
		for(int C=0; C<N-1; ++C){
			for(int R=0; R<N-1; ++R){
				(*this)(R,C) *= amount[C];
			}
		}
		return *this;
	}

	/// Scale transformation matrix by uniform amount
	template<class V>
	Mat& scale(const V& amount){ return scale(Vec<N-1,V>(amount)); }

	/// Scale transformation matrix global coordinates
	template<class V>
	Mat& scaleGlobal(const Vec<N-1,V>& amount){
		for(int R=0; R<N-1; ++R){
			for(int C=0; C<N-1; ++C){
				(*this)(R,C) *= amount[R];
			}
		}
		return *this;
	}

	/// Translate transformation matrix
	template<class V>
	Mat& translate(const Vec<N-1,V>& amount){
		for(int R=0; R<N-1; ++R){
			(*this)(R, N-1) += amount[R];
		}
		return *this;
	}

};



// -----------------------------------------------------------------------------
// The following are functions that either cannot be defined as class methods
// (due to syntax rules or specialization) or simply are not object oriented.

// Non-member binary arithmetic operations
template <int N, class T>
inline Mat<N,T> operator + (const T& s, const Mat<N,T>& v){ return  v+s; }

template <int N, class T>
inline Mat<N,T> operator - (const T& s, const Mat<N,T>& v){ return -v+s; }

template <int N, class T>
inline Mat<N,T> operator * (const T& s, const Mat<N,T>& v){ return  v*s; }


// Basic Vec/Mat Arithmetic
template <int N, class T>
inline Vec<N,T> operator* (const Mat<N,T>& m, const Vec<N,T>& vCol){
	Vec<N,T> r; return Mat<N,T>::multiply(r, m,vCol);
}

template <int N, class T>
inline Vec<N,T> operator* (const Vec<N,T>& vRow, const Mat<N,T>& m){
	Vec<N,T> r; return Mat<N,T>::multiply(r, vRow,m);
}


// TODO: Determinants of higher order matrices:
//			For small N, find recursively using determinants of minors
//			For large N, find using Gaussian elimination
//				(product of diagonal terms in row echelon form)
//				We need a rowEchelon() method for Mat (should indicate what rows were swapped, if any)

/// Get determinant of 1-by-1 matrix
template <class T>
T determinant(const Mat<1,T>& m){
	return m(0,0);
}

/// Get determinant of 2-by-2 matrix
template <class T>
T determinant(const Mat<2,T>& m){
	return m(0,0)*m(1,1) - m(0,1)*m(1,0);
}

/// Get determinant of 3-by-3 matrix
template <class T>
T determinant(const Mat<3,T>& m){
	return
		m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) +
		m(0,1)*(m(1,2)*m(2,0) - m(1,0)*m(2,2)) +
		m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));
}


/// TODO: general Mat inversion, need general determinant first

/// Invert 2-by-2 matrix, returns whether matrix was able to be inverted
template <class T>
bool invert(Mat<2,T>& m){
	T det = determinant(m);
	if(det != 0){
		m.set(
			 m(1,1),-m(0,1),
			-m(1,0), m(0,0)
		) /= det;
		return true;
	}
	return false;
}

/// Invert 3-by-3 matrix, returns whether matrix was able to be inverted
template <class T>
bool invert(Mat<3,T>& m){
	T det = determinant(m);
	if(det != 0){
		m.transpose() /= det;
		return true;
	}
	return false;
}

/// Print
template <int N, class T> 
static void print(const Mat<N,T>& m) {
	for(int R=0; R<N; ++R){
		for(int C=0; C<N; ++C){
			printf("% 6.3g ", double(m(R,C)));
		}	printf("\n");
	}	printf("\n");
}

#undef IT
} // al::
#endif
