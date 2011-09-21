#ifndef INCLUDE_AL_VEC_HPP
#define INCLUDE_AL_VEC_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Generic n-vector and n-by-n matrix classes

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/


#include <cmath>
#include <stdio.h>
#include <ostream>

#include "allocore/math/al_Functions.hpp"

namespace al {


template <int N, class T> class Vec;
template <int N, class T> class Mat;

typedef Vec<2,float>	Vec2f;	///< float 2-vector
typedef Vec<2,double>	Vec2d;	///< double 2-vector
typedef Vec<2,int>		Vec2i;	///< integer 2-vector
typedef Vec<3,float>	Vec3f;	///< float 3-vector
typedef Vec<3,double>	Vec3d;	///< double 3-vector
typedef Vec<3,int>		Vec3i;	///< integer 3-vector
typedef Vec<4,float>	Vec4f;	///< float 4-vector
typedef Vec<4,double>	Vec4d;	///< double 4-vector
typedef Vec<4,int>		Vec4i;	///< integer 4-vector
typedef Mat<3,float>	Mat3f;	///< float 3x3 matrix
typedef Mat<3,double>	Mat3d;	///< double 3x3 matrix
typedef Mat<3,int>		Mat3i;	///< integer 3x3 matrix
typedef Mat<4,float>	Mat4f;	///< float 4x4 matrix
typedef Mat<4,double>	Mat4d;	///< double 4x4 matrix
typedef Mat<4,int>		Mat4i;	///< integer 4x4 matrix

// Forward iterates from 0 to n-1. Current index is 'i'.
#define IT(n) for(int i=0; i<(n); ++i)

template <int N, class T>
struct VecElems{ T x,y,z,w; private: T data[N-4]; };
template<class T> struct VecElems<0,T>{};
template<class T> struct VecElems<1,T>{ T x; };
template<class T> struct VecElems<2,T>{ T x,y; };
template<class T> struct VecElems<3,T>{ 
	T x,y,z; 
	
	/// methods that only make sense for Vec<3,T>:
	
	/// returns cross product of this x b
	inline Vec<3,T> cross(const Vec<3,T>& b) const {
		return Vec<3,T>( y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x );
	}
};
template<class T> struct VecElems<4,T>{ T x,y,z,w; };


/// N-vector

/// This is a fixed size array to enable better loop unrolling optimizations
/// by the compiler and to avoid an extra 'size' data member for small-sized
/// arrays.
template <int N, class T>
class Vec : public VecElems<N,T>{
public:
	using VecElems<N,T>::x;

	typedef T value_type; 


	/// @param[in] v		value to initialize all elements to
	Vec(const T& v=T()){ set(v); }
	
	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	Vec(const T& v1, const T& v2){ set(v1, v2); }
	
	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	/// @param[in] v3		value to initialize third element
	Vec(const T& v1, const T& v2, const T& v3){ set(v1, v2, v3); }
	
	/// @param[in] v1		value to initialize first element
	/// @param[in] v2		value to initialize second element
	/// @param[in] v3		value to initialize third element
	/// @param[in] v4		value to initialize fourth element
	Vec(const T& v1, const T& v2, const T& v3, const T& v4){ set(v1, v2, v3, v4); }

	/// @param[in] v		vector to initialize all elements to
	template <class T2>
	Vec(const Vec<N, T2>& v){ set(v); }

	/// @param[in] v		vector to initialize all elements to
	/// @param[in] s		value of last element
	template <class Tv, class Ts>
	Vec(const Vec<N-1, Tv>& v, const Ts& s){ set(v,s); }

	/// @param[in] v		pointer to array to initialize all elements to
	/// @param[in] stride	stride factor through array
	template <class T2>
	Vec(const T2 * v, int stride=1){ set(v,stride); }



	//--------------------------------------------------------------------------
	// Memory Operations

	/// Returns number of elements
	static int size(){ return N; }

	/// Get read-only pointer to elements
	const T * elems() const { return &x; }

	/// Get read-write pointer to elements
	T * elems(){ return &x; }

	/// Set element at index with no bounds checking
	T& operator[](int i){ return elems()[i];}
	
	/// Get element at index with no bounds checking
	const T& operator[](int i) const { return elems()[i]; }

	/// Return true if objects are element-wise equal, false otherwise
	bool operator ==(const Vec& v) const { IT(N){ if((*this)[i] != v[i]) return false; } return true; }
	
	/// Return true if all elements are equal to value, false otherwise
	bool operator ==(const T& v) const { IT(N){ if((*this)[i] != v   ) return false; } return true; }

	/// Return true if objects are not element-wise equal, false otherwise
	bool operator !=(const Vec& v) const { return !(*this == v); }
	
	/// Return true if all elements are not equal to value, false otherwise
	bool operator !=(const T& v) const { return !(*this == v); }

	/// Get a vector comprised of indexed elements
	Vec<2,T> get(int i0, int i1) const {
		return Vec<2,T>((*this)[i0], (*this)[i1]); }

	/// Get a vector comprised of indexed elements
	Vec<3,T> get(int i0, int i1, int i2) const {
		return Vec<3,T>((*this)[i0], (*this)[i1], (*this)[i2]); }

	/// Get a vector comprised of indexed elements
	Vec<4,T> get(int i0, int i1, int i2, int i3) const {
		return Vec<4,T>((*this)[i0], (*this)[i1], (*this)[i2], (*this)[i3]); }

	/// Get a subvector
	template <int M>
	Vec<M,T> sub(int begin=0) const {
		return Vec<M,T>(elems()+begin);
	}



	//--------------------------------------------------------------------------
	// Basic Arithmetic Operations
	
	Vec& operator  =(const Vec& v){ return set(v); }
	Vec& operator  =(const   T& v){ return set(v); }
	Vec& operator +=(const Vec& v){ IT(N) (*this)[i] += v[i]; return *this; }
	Vec& operator +=(const   T& v){ IT(N) (*this)[i] += v;    return *this; }
	Vec& operator -=(const Vec& v){ IT(N) (*this)[i] -= v[i]; return *this; }
	Vec& operator -=(const   T& v){ IT(N) (*this)[i] -= v;    return *this; }
	Vec& operator *=(const Vec& v){ IT(N) (*this)[i] *= v[i]; return *this; }
	Vec& operator *=(const   T& v){ IT(N) (*this)[i] *= v;    return *this; }
	Vec& operator /=(const Vec& v){ IT(N) (*this)[i] /= v[i]; return *this; }
	Vec& operator /=(const   T& v){ IT(N) (*this)[i] /= v;    return *this; }

	Vec operator + (const Vec& v) const { return Vec(*this) += v; }
	Vec operator + (const   T& v) const { return Vec(*this) += v; }
	Vec operator - (const Vec& v) const { return Vec(*this) -= v; }
	Vec operator - (const   T& v) const { return Vec(*this) -= v; }
	Vec operator * (const Vec& v) const { return Vec(*this) *= v; }
	Vec operator * (const   T& v) const { return Vec(*this) *= v; }
	Vec operator / (const Vec& v) const { return Vec(*this) /= v; }
	Vec operator / (const   T& v) const { return Vec(*this) /= v; }
	Vec operator - () const { return Vec(*this).negate(); }

	/// Set elements from another vector
	template <class T2>
	Vec& set(const Vec<N, T2> &v){ IT(N){ (*this)[i] = T(v[i]); } return *this; }

	/// Set elements from another vector
	template <int N2, class T2>
	Vec& set(const Vec<N2, T2> &v){ IT(N<N2?N:N2){ (*this)[i] = T(v[i]); } return *this; }

	/// Set elements from another vector and scalar
	template <class Tv, class Ts>
	Vec& set(const Vec<N-1, Tv> &v, const Ts& s){ (*this)[N-1]=s; return set(v); }

	/// Set all elements to the same value
	Vec& set(const T& v){ IT(N){ (*this)[i] = v; } return *this; }

	/// Set elements from raw C-pointer
	template <class T2>
	Vec& set(const T2 * v){
		IT(N){ (*this)[i] = T(v[i]); }
		return *this;
	}
	
	/// Set elements from strided raw C-pointer
	template <class T2>
	Vec& set(const T2 * v, int stride){
		IT(N){ (*this)[i] = T(v[i*stride]); }
		return *this;
	}

	/// Set first 2 elements
	Vec& set(const T& v1, const T& v2){
		return set(v1,v2,v1,v1,v1,v1); }

	/// Set first 3 elements
	Vec& set(const T& v1, const T& v2, const T& v3){
		return set(v1,v2,v3,v1,v1,v1); }

	/// Set first 4 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4){
		return set(v1,v2,v3,v4,v1,v1); }

	/// Set first 5 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5){
		return set(v1,v2,v3,v4,v5,v1); }

	/// Set first 6 elements
	Vec& set(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6){		
		switch(N){
		default:(*this)[5] = v6;
		case 5: (*this)[4] = v5;
		case 4: (*this)[3] = v4;
		case 3: (*this)[2] = v3;
		case 2: (*this)[1] = v2;
		case 1: (*this)[0] = v1;
		}
		return *this;
	}

	/// Set all elements to zero
	Vec& zero(){ return set(T(0)); }

	
	/// Clip to range:
	/// NOTE argument order (max,min)
	Vec& clip(T max=T(1), T min=T(0)) {
		IT(N) (*this)[i] = al::clip((*this)[i], max, min);
		return *this;
	}
	Vec& clip(Vec max, Vec min) {
		IT(N) (*this)[i] = al::clip((*this)[i], max[i], min[i]);
		return *this;
	}
	
	/// Wrap in range:
	/// NOTE argument order (max,min)
	Vec& wrap(T max=T(1), T min=T(0)) {
		IT(N) (*this)[i] = al::wrap((*this)[i], max, min);
		return *this;
	}
	Vec& wrap(Vec max, Vec min) {
		IT(N) (*this)[i] = al::wrap((*this)[i], max[i], min[i]);
		return *this;
	}
	
	/// Fold in range:
	/// NOTE argument order (max,min)
	Vec& fold(T max=T(1), T min=T(0)) {
		IT(N) (*this)[i] = al::fold((*this)[i], max, min);
		return *this;
	}
	Vec& fold(Vec max, Vec min) {
		IT(N) (*this)[i] = al::fold((*this)[i], max[i], min[i]);
		return *this;
	}


	//--------------------------------------------------------------------------
	// Linear Operations

	/// Returns dot (inner) product between vectors
	T dot(const Vec& v) const {
		T r = (*this)[0] * v[0];
		for(int i=1; i<N; ++i){ r += (*this)[i] * v[i]; }
		return r;
	}
	
	/// Returns magnitude
	T mag() const { return std::sqrt(magSqr()); }
	
	/// Returns magnitude squared
	T magSqr() const { return dot(*this); }
	
	/// Returns p-norm of elements
	
	/// The p-norm is pth root of the sum of the absolute value of the elements 
	/// raised to the pth, (sum |x_n|^p) ^ (1/p).
	T norm(const T& p) const {
		using namespace std;
		T r = pow(abs((*this)[0]), p);
		for(int i=1; i<N; ++i){ r += pow(abs((*this)[i]), p); }
		return pow(r, T(1)/p);		
	}

	/// Return 1-norm (sum) of elements
	T norm1() const { return sumAbs(); }
	
	/// Return 2-norm of elements
	T norm2() const { return mag(); }

	/// Returns product of elements
	T product() const {
		T r = (*this)[0];
		for(int i=1; i<N; ++i){ r *= (*this)[i]; }
		return r;
	}
	
	/// Reflect vector around line
	Vec reflect(const Vec& normal) {
		return this - (2 * dot(normal) * normal);
	}

	/// Returns closest vector on unit N-sphere
	Vec sgn() const { return Vec(*this).normalize(); }

	/// Returns sum of elements
	T sum() const {
		T r = (*this)[0];
		for(int i=1; i<N; ++i){ r += (*this)[i]; }
		return r;
	}

	/// Returns sum of absolute value of elements
	T sumAbs() const {
		T r = abs((*this)[0]);
		for(int i=1; i<N; ++i){ r += abs((*this)[i]); }
		return r;
	}

	/// Negates all elements
	Vec& negate(){ IT(N){ (*this)[i] = -(*this)[i]; } return *this; }

	/// Scales elements evenly so magnitude is one
	Vec& normalize();
	
	/// linear interpolation
	void lerp(const Vec& target, T amt) { set(lerp(*this, target, amt)); }
	static Vec lerp(const Vec& input, const Vec& target, T amt) {
		return input+amt*(target-input);
	}
	
	/// debug printing
	void print(FILE * out=stdout) const;
	
};



/// N-by-N square matrix

/// Elements are stored in column-major format.
///
template <int N, class T>
class Mat{
public:

	/// Column-major array of elements
	T mElems[N*N];

	Mat(){ set(T(0)); }

	/// param[in] arr	one dimensional array in column-major
	Mat(const T * arr){ set(arr); }

	static struct NoInit{} NOINIT;
	Mat(const NoInit& v){}

	///
	Mat(
		const T& r1c1, const T& r1c2,
		const T& r2c1, const T& r2c2
	){
		set(r1c1, r1c2,
			r2c1, r2c2
		);
	}

	///
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

	///
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
		return Mat(NOINIT).setIdentity();
	}

	/// Get a rotation transform matrix
	static Mat rotation(double angle, int dim1, int dim2){
		double cs = cos(angle);
		double sn = sin(angle);

		Mat m(NOINIT);
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
		Mat m(NOINIT);
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

	Mat operator - () const { Mat r; IT(size()){ r[i]=-(*this)[i]; } return r; }
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
	Mat& scale(const V& amount){
		for(int C=0; C<N-1; ++C){
			for(int R=0; R<N-1; ++R){
				(*this)(R,C) *= amount;
			}
		}
		return *this;
	}

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

// Vec
template <int N, class T>
inline Vec<N,T> operator + (T s, const Vec<N,T>& v){ return  v+s; }

template <int N, class T>
inline Vec<N,T> operator - (T s, const Vec<N,T>& v){ return -v+s; }

template <int N, class T>
inline Vec<N,T> operator * (T s, const Vec<N,T>& v){ return  v*s; }

template <int N, class T>
inline Vec<N,T> operator / (T s, const Vec<N,T>& v){
	Vec<N,T> r; IT(N){ r[i] = s/v[i]; } return r;
}

/// Get absolute value (magnitude) of vector
template <int N, class T>
inline T abs(const Vec<N,T>& v){ return v.mag(); }


// Mat
template <int N, class T>
inline Mat<N,T> operator + (T s, const Mat<N,T>& v){ return  v+s; }

template <int N, class T>
inline Mat<N,T> operator - (T s, const Mat<N,T>& v){ return -v+s; }

template <int N, class T>
inline Mat<N,T> operator * (T s, const Mat<N,T>& v){ return  v*s; }


// Basic Vec/Mat Arithmetic
template <int N, class T>
inline Vec<N,T> operator* (const Mat<N,T>& m, const Vec<N,T>& vCol){
	Vec<N,T> r; return Mat<N,T>::multiply(r, m,vCol);
}

template <int N, class T>
inline Vec<N,T> operator* (const Vec<N,T>& vRow, const Mat<N,T>& m){
	Vec<N,T> r; return Mat<N,T>::multiply(r, vRow,m);
}


// Specialized vector functions

/// Returns concatenation of two vectors
template <int N1, class T1, int N2, class T2>
inline Vec<N1+N2, T1> concat(const Vec<N1,T1>& a, const Vec<N2,T2>& b){
	Vec<N1+N2, T1> r;
	r.set(a.elems());
	for(int i=0; i<N2; ++i) r[i+N1] = T1(b[i]);
	return r;
}

/// Sets r to cross product, a x b
template <class T>
inline void cross(Vec<3,T>& r, const Vec<3,T>& a, const Vec<3,T>& b){
//	r[0] = a[1]*b[2] - a[2]*b[1];
//	r[1] = a[2]*b[0] - a[0]*b[2];
//	r[2] = a[0]*b[1] - a[1]*b[0];
	r = a.cross(b);
}

/// Returns cross product, a x b
template <class T>
inline Vec<3,T> cross(const Vec<3,T>& a, const Vec<3,T>& b){
	Vec<3,T> r;	cross(r,a,b); return r;
}

/// Returns angle, in interval [0, pi], between two vectors
template <int N, class T>
static T angle(const Vec<N,T>& a, const Vec<N,T>& b){
	return std::acos(a.sgn().dot(b.sgn()));
}

/*! Centroid of a triangle defined by three points
	@param p1	Point1
	@param p2	Point2
	@param p3	Point3
	@ret c		Centroid
*/
template <int N, class T>
inline void centroid3(Vec<N,T>& c, const Vec<N,T>& p1, const Vec<N,T>& p2, const Vec<N,T>& p3){
	static const T _1_3 = T(1)/T(3);
	c = (p1+p2+p3)*_1_3;
}

/*! Get the normal to a triangle defined by three points
	@param p1	Point1
	@param p2	Point2
	@param p3	Point3
	@ret n		Normal
*/
template <class T>
inline void normal(Vec<3,T>& n, const Vec<3,T>& p1, const Vec<3,T>& p2, const Vec<3,T>& p3){
	cross(n, p2-p1, p3-p1);	
	n.normalize();
}

/// Returns vector containing element-wise minimum between two vectors
template <int N, class T>
inline Vec<N,T> min(const Vec<N,T>& a, const Vec<N,T>& b){
	Vec<N,T> r;
	IT(N){ r[i] = a[i] > b[i] ? b[i] : a[i]; }
	return r;
}

/// Returns vector containing element-wise maximum between two vectors
template <int N, class T>
inline Vec<N,T> max(const Vec<N,T>& a, const Vec<N,T>& b){	
	Vec<N,T> r;
	IT(N){ r[i] = a[i] < b[i] ? b[i] : a[i]; }
	return r;
}


// Specialized MatN functions

// TODO: Determinants of higher order matrices:
//			For small N, find recursively using determinants of minors
//			For large N, find using Gaussian elimination
//				(product of diagonal terms in row echelon form)
//				We need a rowEchelon() method for Mat (should indicate what rows were swapped, if any)

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
static inline void print(const Mat<N,T>& m) {
	for(int R=0; R<N; ++R){
		for(int C=0; C<N; ++C){
			printf("% 6.3g ", double(m(R,C)));
		}	printf("\n");
	}	printf("\n");
}


// Implementation --------------------------------------------------------------

template<int N, class T>
inline void Vec<N,T>::print(FILE * out) const {
	fprintf(out, "Vec%i(%f", N,  (double)((*this)[0]));
	for (int C=1; C<N; C++) 
		fprintf(out, ", %f", (double)((*this)[C]));
	fprintf(out, ")\n");
}

template <int N, class T>
Vec<N,T>& Vec<N,T>::normalize(){
	float m = mag();
	if(m > T(1e-20)){
		(*this) /= m;
	}
	else{
		set(T(0));
		(*this)[0] = T(1);
	}
	return *this;
}


// Pretty-printing by Matt:
//
template <typename T> const char* typeString();
#define TypeString(A) template <> inline const char* typeString<A>() { return #A; }
TypeString(char)
TypeString(unsigned char)
TypeString(int)
TypeString(unsigned int)
TypeString(float)
TypeString(double)
TypeString(long double)
#undef TypeString

template <int N, class T>
std::ostream & operator << (std::ostream & out, const Vec<N,T> &v) {
  out << "Vec<" << N << "," << typeString<T>() << "> = {" << v.elems()[0];
  for (int i = 1; i < N; ++i)
    out << ", " << v.elems()[i];
  out << "}" << std::endl;
  return out;
}

#undef IT

} // ::al::

#endif /* include guard */
