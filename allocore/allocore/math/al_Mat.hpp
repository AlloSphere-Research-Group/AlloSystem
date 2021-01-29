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

typedef Mat<2,float>	Mat2f;	///< float 2x2 matrix
typedef Mat<2,double>	Mat2d;	///< double 2x2 matrix
typedef Mat<2,int>		Mat2i;	///< integer 2x2 matrix
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
/// @ingroup allocore
template <int N, class T>
class Mat{
public:

	typedef T value_type;

	/// Column-major array of elements
	T mElems[N*N];


	//--------------------------------------------------------------------------
	// Constructors

	/// Default constructor that initializes elements to zero
	Mat(){ set(T(0)); }

	/// Construct without initializing elements
	Mat(const MatNoInit& v){}

	/// @param[in] arr	one dimensional array in column-major
	template <class U>
	Mat(const U * arr){ set(arr); }

	/// @param[in] src	matrix with same dimension, but possibly different type
	template <class U>
	Mat(const Mat<N,U>& src){ set(src.elems()); }

	template <int M, class U>
	Mat(const Mat<M,U>& src){
		*this = src;
	}

	/// Create diagonal matrix

	/// Sets the diagonal elements to the input value and all other elements
	/// to zero. The identity matrix is created by passing in a value of 1.
	/// @param[in] diag		The value to place on the diagonal
	Mat(const T& diag){
		diagonal(diag);
	}

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
		return Mat(T(1));
	}

	/// Get a rotation transform matrix

	/// @param[in] angle	rotation angle in radians
	/// @param[in] dim1		first basis vector of rotation plane
	/// @param[in] dim2		second basis vector of rotation plane
	static Mat rotation(double angle, int dim1, int dim2){
		double cs = cos(angle);
		double sn = sin(angle);
		Mat m(T(1));
		m(dim1,dim1) = cs;
		m(dim1,dim2) =-sn;
		m(dim2,dim1) = sn;
		m(dim2,dim2) = cs;
		return m;
	}

	/// Get a rotation transform matrix

	/// @param[in] angle	rotation angle in radians
	/// @param[in] axis		rotation axis; should be a unit vector
	static Mat<4,T> rotation(double angle, const Vec<3,T>& axis){
		T s = sin(angle);
		T c = cos(angle);
		T t = T(1)-c;
		T x = axis[0], y = axis[1], z = axis[2];
		T tx = t*x, ty = t*y, tz = t*z;
		T sx = s*x, sy = s*y, sz = s*z;

		return Mat<4,T>(
			tx*x + c , tx*y - sz, tx*z + sy, T(0),
			tx*y + sz, ty*y + c , ty*z - sx, T(0),
			tx*z - sy, ty*z + sx, tz*z + c , T(0),
			T(0),T(0),T(0),T(1)
		);
	}

	/// Get a scaling transform matrix
	template <class V>
	static Mat scaling(const Vec<N-1,V>& v){
		Mat m;
		for(int r=0; r<N-1; ++r) m(r,r) = v[r];
		m(N-1,N-1) = T(1);
		return m;
	}

	/// Get a scaling transform matrix
	template <class V>
	static Mat scaling(V v){
		return scaling(Vec<N-1,V>(v));
	}

	/// Get a scaling transform matrix
	template <typename... Vals>
	static Mat scaling(Vals... vals){
		return scaling(Vec<(sizeof...(Vals)),T>(vals...));
	}

	/// Get a translation transform matrix
	template <class V>
	static Mat translation(const Vec<N-1,V>& v){
		Mat m(T(1));
		for(int r=0; r<N-1; ++r) m(r,N-1) = v[r];
		return m;
	}

	/// Get a translation transform matrix
	template <typename... Vals>
	static Mat translation(Vals... vals){
		return translation(Vec<(sizeof...(Vals)),T>(vals...));
	}

	//--------------------------------------------------------------------------
	// Memory Operations

	/// Returns C array type punned into a matrix
	static Mat& pun(T * src){ return *(Mat*)(src); }
	static const Mat& pun(const T * src){ return *(const Mat*)(src); }

	/// Returns total number of elements
	static constexpr int size(){ return N*N; }

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

	template <int Row, int Col>
	T& at(){
		static_assert(Row<N && Col<N, "Row or column out of bounds");
		constexpr auto idx = Col*N+Row;
		return mElems[idx];
	}

	template <int Row, int Col>
	const T& at() const {
		static_assert(Row<N && Col<N, "Row or column out of bounds");
		constexpr auto idx = Col*N+Row;
		return mElems[idx];
	}

	/// Return column i as vector
	const Vec<N,T>& col(int i) const { return Vec<N,T>::pun(elems() + i*N); }
	Vec<N,T>& col(int i){ return Vec<N,T>::pun(elems() + i*N); }

	template <unsigned Index>
	const Vec<N,T>& col() const {
		static_assert(Index < N, "Column index out of bounds");
		return col(Index);
	}

	template <unsigned Index>
	Vec<N,T>& col(){
		static_assert(Index < N, "Column index out of bounds");
		return col(Index);
	}

	/// Return row i as vector
	Vec<N,T> row(int i) const { return Vec<N,T>(elems()+i, N); }

	template <unsigned Index>
	Vec<N,T> row() const {
		static_assert(Index < N, "Row index out of bounds");
		return row(Index);
	}

	/// Return diagonal
	Vec<N,T> diagonal() const { return Vec<N,T>(elems(), N+1); }

	/// Transpose elements

	/// @param[in] size		Size of upper-left submatrix to transpose
	///
	Mat& transpose(int size=N){
		for(int j=0; j<size-1; ++j){	// row and column
		for(int i=j+1; i<size; ++i){	// offset into row or column
			std::swap((*this)(i,j), (*this)(j,i));
		}}
		return *this;
	}

	/// Get an MxM submatrix
	template <int M>
	Mat<M,T> sub(int row=0, int col=0) const {
		static_assert(M<=N, "Submatrix size cannot be larger than matrix size");
		Mat<M,T> res(MAT_NO_INIT);
		for(int j=0; j<M; ++j){
		for(int i=0; i<M; ++i){
			res(j,i) = (*this)(j+row, i+col);
		}}
		return res;
	}

	/// Returns a submatrix by removing one row and column
	Mat<N-1,T> submatrix(int row, int col) const {
		Mat<N-1,T> res(MAT_NO_INIT);
		for(int j=0,js=0; j<N-1; ++j, ++js){
			js += int(js==row);
			for(int i=0, is=0; i<N-1; ++i, ++is){
				is += int(is==col);
				res(j,i) = (*this)(js,is);
			}
		}
		return res;
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
	template <class U>
	static Vec<N,U>& multiply(Vec<N,U>& r, const Mat& m, const Vec<N,U>& vCol){
		IT(N){ r[i] = m.row(i).dot(vCol); }
		return r;
	}

	/// Computes product of row vector multiplied by matrix, r = vRow * m
	template <class U>
	static Vec<N,U>& multiply(Vec<N,U>& r, const Vec<N,U>& vRow, const Mat& m){
		IT(N){ r[i] = vRow.dot(m.col(i)); }
		return r;
	}

	/// Transform a point by this matrix treated as a homogenous transform
	template <class U>
	Vec<N-1,U> transformPoint(const Vec<N-1,U>& v){
		Vec<N-1,U> r;
		IT(N-1){
			auto mrow = row(i);
			r[i] = mrow.template sub<N-1>().dot(v) + mrow[N-1];
		}
		return r;
	}

	/// Transform a vector by this matrix treated as a homogenous transform
	template <class U>
	Vec<N-1,U> transformVector(const Vec<N-1,U>& v){
		Vec<N-1,U> r;
		IT(N-1){ r[i] = row(i).template sub<N-1>().dot(v); }
		return r;
	}
	

	/// Set elements from different sized matrix

	/// Only the corresponding elements are copied from the source. Extra
	/// elements in the destination are set according to the identity matrix.
	template <int M, class U>
	Mat& operator = (const Mat<M,U>& v){
		setIdentity(); // this could perhaps be done more efficiently
		constexpr auto L = N<M ? N : M;
		for(int r=0; r<L; ++r)
			for(int c=0; c<L; ++c)
				(*this)(c,r) = v(c,r);
		return *this;
	}

	/// Set all elements to value
	Mat& set(const T& v){ IT(size()){ (*this)[i]=v; } return *this; }

	/// Set elements from another matrix
	template <class U>
	Mat& set(const Mat<N,U>& v){ return set(v.elems()); }

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
		static_assert(N>=2, "Attempt to set matrix elements out of bounds");
		(*this)(row  , col) = v1;
		(*this)(row+1, col) = v2; return *this;
	}

	/// Set a (sub)column
	Mat& setCol3(const T& v1, const T& v2, const T& v3, int col=0, int row=0){
		static_assert(N>=3, "Attempt to set matrix elements out of bounds");
		(*this)(row  , col) = v1;
		(*this)(row+1, col) = v2;
		(*this)(row+2, col) = v3; return *this;
	}

	/// Set a (sub)column
	Mat& setCol4(const T& v1, const T& v2, const T& v3, const T& v4, int col=0, int row=0){
		static_assert(N>=4, "Attempt to set matrix elements out of bounds");
		(*this)(row,   col) = v1;
		(*this)(row+1, col) = v2;
		(*this)(row+2, col) = v3;
		(*this)(row+3, col) = v4; return *this;
	}

	/// Set elements on diagonal to one and all others to zero
	Mat& setIdentity(){
		return diagonal(T(1));
	}

	/// Set elements on diagonal to a value and all others to zero
	Mat& diagonal(T v){
		for(int i=0; i<N; ++i)
			(*this)[i*(N+1)] = v;

		for(int i=0;   i<N-1  ; ++i){
		for(int j=i+1; j<N+i+1; ++j){
			(*this)[i*N + j] = T(0);
		}}
		return *this;
	}


	//--------------------------------------------------------------------------
	// Linear Operations


	/// Get cofactor
	T cofactor(int row, int col) const {
		T minor = determinant(submatrix(row,col));
		T cofactors[] = {minor, -minor};
		// cofactor sign: + if row+col even, - otherwise
		int sign = (row^col) & 1;
		return cofactors[sign];
	}

	/// Get cofactor matrix
	Mat<N,T> cofactorMatrix() const {
		Mat<N,T> res(MAT_NO_INIT);
		for(int r=0; r<N; ++r){
			for(int c=0; c<N; ++c){
				res(r,c) = cofactor(r,c);
			}
		}
		return res;
	}

	/// Get trace (sum of diagonal elements)
	T trace() const { return diagonal().sum(); }

	/// Extract rotation part of transformation matrix
	Mat<N-1,T> rotation() const {
		auto R = sub<N-1>();
		for(int i=0; i<N-1; ++i) R.col(i).normalize();
		return R;
	}


	// Affine transformations

	/// Rotate transformation matrix on a local plane (A' = AR)

	/// @param[in] angle	angle of rotation, in radians
	/// @param[in] dim1		local coordinate frame axis to rotate away from
	/// @param[in] dim2		local coordinate frame axis to rotate towards
	Mat& rotate(double angle, int dim1, int dim2){
		return rotate(cos(angle), sin(angle), dim1, dim2);
	}

	Mat& rotate(double cosAngle, double sinAngle, int dim1, int dim2){
		for(int R=0; R<N-1; ++R){
			const T& v1 = (*this)(R, dim1);
			const T& v2 = (*this)(R, dim2);
			T t = v1*cosAngle + v2*sinAngle;
			(*this)(R, dim2) = v2*cosAngle - v1*sinAngle;
			(*this)(R, dim1) = t;
		}
		return *this;
	}

	template <int Dim1, int Dim2>
	Mat& rotate(double angle){
		static_assert(Dim1<N-1 && Dim2<N-1, "Dimension out of bounds");
		return rotate(angle, Dim1, Dim2);
	}

	template <int Dim1, int Dim2>
	Mat& rotate(double cosAngle, double sinAngle){
		return rotate<Dim1,Dim2>(cosAngle, sinAngle);
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

	/// Scale transformation matrix
	template<typename... Vals>
	Mat& scale(Vals... vals){ return scale(Vec<(sizeof...(Vals)),T>(vals...)); }

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

	/// Translate transformation matrix by same amount in all directions
	template<class V>
	Mat& translate(const V& amount){ return translate(Vec<N-1,V>(amount)); }

	/// Translate transformation matrix
	template<typename... Vals>
	Mat& translate(Vals... vals){ return translate(Vec<(sizeof...(Vals)),T>(vals...)); }

	/// Print to file (stream)
	void print(FILE * file = stderr) const;
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
template <int N, class T, class U>
inline Vec<N,U> operator* (const Mat<N,T>& m, const Vec<N,U>& vCol){
	Vec<N,U> r; return Mat<N,T>::multiply(r, m,vCol);
}

template <int N, class T, class U>
inline Vec<N,U> operator* (const Vec<N,U>& vRow, const Mat<N,T>& m){
	Vec<N,U> r; return Mat<N,T>::multiply(r, vRow,m);
}

/// Get determinant

/// This computes the determinant using cofactor (or Laplace) expansion.
/// The algorithm operates by recursively computing determinants of submatrices.
/// For small matrices, optimized versions are used.
/// @ingroup allocore
template<int N, class T>
T determinant(const Mat<N,T>& m){
	T res = 0;
	for(int i=0; i<N; ++i){
		T entry = m(0,i);
		if(entry != T(0)){
			res += entry * m.cofactor(0,i);
		}
	}
	return res;
}

template <class T>
T determinant(const Mat<1,T>& m){
	return m(0,0);
}

template <class T>
T determinant(const Mat<2,T>& m){
	return m(0,0)*m(1,1) - m(0,1)*m(1,0);
}

template <class T>
T determinant(const Mat<3,T>& m){
	return
		m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) +
		m(0,1)*(m(1,2)*m(2,0) - m(1,0)*m(2,2)) +
		m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0));
}

/// Invert matrix, returns whether matrix was able to be inverted
/// For small matrices, optimized versions are used.
/// @ingroup allocore
template<int N, class T>
bool invert(Mat<N,T>& m){
	// Get cofactor matrix, C
	Mat<N,T> C = m.cofactorMatrix();

	// Compute determinant
	T det = T(0);
	for(int i=0; i<N; ++i){
		det += m(0,i) * C(0,i);
	}

	// Divide adjugate matrix, C^T, by determinant
	if(det != T(0)){
		m = (C.transpose() *= T(1)/det);
		return true;
	}

	return false;
}

template <class T>
bool invert(Mat<1,T>& m){
	T det = determinant(m);
	if(det != 0){
		m(0,0) /= det;
		return true;
	}
	return false;
}

template <class T>
bool invert(Mat<2,T>& m){
	T det = determinant(m);
	if(det != T(0)){
		m = Mat<2,T>(
			 m(1,1),-m(0,1),
			-m(1,0), m(0,0)
		) *= T(1)/det;
		return true;
	}
	return false;
}

template <class T>
bool invert(Mat<3,T>& m){
	auto nx = cross(m.col(1), m.col(2));
	auto ny = cross(m.col(2), m.col(0));
	auto nz = cross(m.col(0), m.col(1));
	auto det= m(0,0)*nx.x + m(1,0)*ny.x + m(2,0)*nz.x;
	if(det != T(0)){
		m.set(
			nx.x, nx.y, nx.z,
			ny.x, ny.y, ny.z,
			nz.x, nz.y, nz.z
		) *= T(1)/det;
		return true;
	}
	return false;
}

/// Invert a proper rigid transformation matrix (rotation + translation)

/// This can be used to convert between camera and view matrices.
///
template <int N, class T>
void invertRigid(Mat<N,T>& m){
	// Given A = T * R, A^-1 = R^-1 * T^-1

	// Compute inverse translation T^-1
	T it[N-1];
	for(int i=0; i<N-1; ++i)
		it[i] = -(m.col(i).dot(m.col(N-1)));

	for(int r=0; r<N-1; ++r)
		m(r,N-1) = it[r];

	// Transpose rotation part to get R^-1
	m.transpose(N-1);
}

/// Get "normal" matrix from modelview

/// This matrix is used to convert normals from object to eye space.
/// It assumes the input matrix is composed of rotation, scaling, and
/// translation. It is normally obtained as the inverse transpose of the
/// modelview, however, this computes it much faster under the given assumption.
template <int N, class T> Mat<N-1,T> normalMatrix(const Mat<N,T>& mv){
	// Given A = S * R,
	//       N = (A^-1)^T
	//         = (R^-1 * S^-1)^T
	//         =  S^-1 * R

	// Compute S^-2
	T iss[N-1];
	for(int i=0; i<N-1; ++i)
		iss[i] = T(1)/sub<N-1>(mv.col(i)).magSqr();

	Mat<N-1,T> res = mv;

	// Apply S^-2
	for(int i=0; i<N-1; ++i)
		res.col(i) *= iss[i];

	return res;
}


/// Get rotation matrix that rotates one unit vector onto another

/// @param[in] from		Unit vector to rotate from
/// @param[in] to		Unit vector to rotate onto
template <class T>
Mat<3,T> rotation(const Vec<3,T>& from, const Vec<3,T>& to){
	// From https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
	auto cosAng = from.dot(to);

	if(cosAng == T(-1)){ // vectors point in opposite directions?
		return Mat<3,T>(T(-1)); // reflection
	}

	auto n = cross(from, to); // normal to plane of rotation

	// Skew-symmetric cross-product matrix: A^T = -A
	/*Mat<3,T> ssc(
		T(0),-n.z,  n.y,
		 n.z, T(0),-n.x,
		-n.y, n.x,  T(0)
	);
	return Mat<3,T>(T(1)) + ssc + ssc*ssc*(T(1)/(T(1)+cosAng));*/

	// Much faster version of above
	auto s = T(1)/(T(1)+cosAng);
	auto sx=s*n.x, sy=s*n.y, sz=s*n.z;
	auto sxx=sx*n.x, sxy=sx*n.y, szx=sx*n.z, syy=sy*n.y, syz=sy*n.z, szz=sz*n.z;
	return Mat<3,T>(
		T(1)-syy-szz, sxy-n.z, szx+n.y,
		sxy+n.z, T(1)-szz-sxx, syz-n.x,
		szx-n.y, syz+n.x, T(1)-sxx-syy
	);
}


//----------------------------
// Member function definitions

template <int N, class T>
void Mat<N,T>::print(FILE * file) const {
	for(int R=0; R<N; ++R){
		fprintf(file, "%c", " {"[R==0]);
		for(int C=0; C<N; ++C){
			fprintf(file, "% 6.3g%s",
				double((*this)(R,C)), (R!=N-1)||(C!=N-1) ? ", " : "}");
		}
		fprintf(file, "\n");
	}
}

#undef IT
} // al::
#endif
